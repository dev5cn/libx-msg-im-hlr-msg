/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <libx-msg-im-hlr-db.h>
#include "XmsgImHlrContractsAddReply.h"

XmsgImHlrContractsAddReply::XmsgImHlrContractsAddReply()
{

}

void XmsgImHlrContractsAddReply::handle(shared_ptr<XmsgNeUsr> nu, SptrUsr usr, SptrClient client, SptrXitp trans, shared_ptr<XmsgImHlrContractsAddReplyReq> req)
{
	auto notice = usr->evn->getNotice(req->ver());
	if (notice == nullptr) 
	{
		trans->endDesc(RET_NOT_FOUND, "can not found transaction for event, ver: %llu", req->ver());
		return;
	}
	if (req->oper() == X_MSG_IM_HLR_CONTRACTS_REPLY_OPER_IGNORE) 
	{
		XmsgImHlrContractsAddReply::eventRead(usr, req); 
		shared_ptr<XmsgImHlrContractsAddReplyRsp> rsp(new XmsgImHlrContractsAddReplyRsp());
		XmsgMisc::insertKv(rsp->mutable_ext(), "accept", "true");
		trans->end(rsp);
		return;
	}
	if (req->oper() != X_MSG_IM_HLR_CONTRACTS_REPLY_OPER_AGREE && req->oper() != X_MSG_IM_HLR_CONTRACTS_REPLY_OPER_REJECT)
	{
		trans->endDesc(RET_FORBIDDEN, "unexpected operation: %s", req->oper().c_str());
		return;
	}
	XmsgImHlrContractsAddReply::eventRead(usr, req); 
	shared_ptr<XmsgImHlrContractsAddReplyRsp> rsp(new XmsgImHlrContractsAddReplyRsp());
	XmsgMisc::insertKv(rsp->mutable_ext(), "accept", "true");
	XmsgImHlrContractsAddNotice an; 
	if (!an.ParseFromString(notice->dat()))
	{
		LOG_DEBUG("parse pb message failed, notice: %s, usr: %s, req: %s", notice->ShortDebugString().c_str(), usr->toString().c_str(), req->ShortDebugString().c_str())
		return;
	}
	SptrCgt cgt = ChannelGlobalTitle::parse(an.cgt());
	if (cgt == nullptr)
	{
		LOG_FAULT("it`s a bug, channel global title format error, usr: %s, req: %s, an: %s", usr->toString().c_str(), req->ShortDebugString().c_str(), an.ShortDebugString().c_str())
		return;
	}
	if (req->oper() == X_MSG_IM_HLR_CONTRACTS_REPLY_OPER_AGREE) 
		XmsgImHlrContractsAddReply::add2contracts(usr, req, cgt);
	auto org = XmsgImUsrMgr::instance()->findXmsgImUsr(an.cgt()); 
	if (org == nullptr)
	{
		LOG_WARN("can not found, notice: %s, usr: %s, req: %s", notice->ShortDebugString().c_str(), usr->toString().c_str(), req->ShortDebugString().c_str())
		return;
	}
	XmsgImHlrDb::instance()->future([usr, req, org] 
	{
		XmsgImHlrContractsAddReply::notice2org4local(usr, req, org);
	});
}

void XmsgImHlrContractsAddReply::add2contracts(SptrUsr usr, shared_ptr<XmsgImHlrContractsAddReplyReq> req, SptrCgt orgCgt)
{
	shared_ptr<XmsgImHlrContractsColl> coll(new XmsgImHlrContractsColl());
	coll->cgt = usr->dat->cgt;
	coll->ctp = orgCgt;
	coll->info.reset(new XmsgKv());
	*(coll->info->mutable_kv()) = req->info(); 
	coll->gts = Xsc::clock;
	coll->uts = coll->gts;
	XmsgImHlrDb::instance()->future([usr, coll] 
	{
		if (!XmsgImHlrContractsCollOper::instance()->insert(coll))
		{
			LOG_ERROR("add to contracts failed, may be database exception, usr: %s, coll: %s", usr->dat->cgt->toString().c_str(), coll->toString().c_str())
			return;
		}
		LOG_DEBUG("add to contracts successful, usr: %s, coll: %s", usr->dat->cgt->toString().c_str(), coll->toString().c_str())
	});
}

void XmsgImHlrContractsAddReply::notice2org4local(SptrUsr usr, shared_ptr<XmsgImHlrContractsAddReplyReq> req, SptrUsr org)
{
	XmsgImHlrContractsAddReplyNotice rn;
	rn.set_cgt(usr->dat->cgt->toString());
	rn.set_oper(req->oper());
	rn.set_desc(req->desc());
	rn.set_ver(req->ver());
	shared_ptr<XmsgImHlrUsrEventColl> coll(new XmsgImHlrUsrEventColl());
	coll->cgt = org->dat->cgt;
	coll->isRead = false;
	coll->ent = XmsgUsrEventNoticeType::TERMINAL_ALL;
	coll->ver = org->evn->nextVer(); 
	coll->msg = XmsgImHlrContractsAddReplyNotice::descriptor()->name();
	coll->dat = rn.SerializeAsString();
	coll->gts = Xsc::clock;
	coll->uts = coll->gts;
	coll->ets = coll->gts + XmsgImHlrCfg::instance()->cfgPb->contracts().usreventexpired() * DateMisc::sec;
	if (!XmsgImHlrUsrEventCollOper::instance()->insert(coll))
	{
		LOG_ERROR("insert usr event in to database failed, usr: %s, req: %s", usr->dat->cgt->toString().c_str(), req->ShortDebugString().c_str())
		return;
	}
	LOG_DEBUG("insert usr event in to database successful, usr: %s, req: %s", usr->dat->cgt->toString().c_str(), req->ShortDebugString().c_str())
	org->future([org, coll]
	{
		org->evn->onEvent(org, coll);
	});
}

void XmsgImHlrContractsAddReply::eventRead(SptrUsr usr, shared_ptr<XmsgImHlrContractsAddReplyReq> req)
{
	usr->evn->eventRead(req->ver()); 
	SptrCgt cgt = usr->dat->cgt;
	XmsgImHlrDb::instance()->future([cgt, req]
	{
		if (!XmsgImHlrUsrEventCollOper::instance()->eventRead(cgt, req->ver()))
		{
			LOG_ERROR("update usr event read flag failed, may be database exception, cgt: %s, req: %s", cgt->toString().c_str(), req->ShortDebugString().c_str())
			return;
		}
		LOG_DEBUG("update usr event read flag successful, cgt: %s, req: %s", cgt->toString().c_str(), req->ShortDebugString().c_str())
	});
}

XmsgImHlrContractsAddReply::~XmsgImHlrContractsAddReply()
{

}

