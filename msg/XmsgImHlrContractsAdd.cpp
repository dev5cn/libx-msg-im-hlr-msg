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
#include "XmsgImHlrContractsAdd.h"

XmsgImHlrContractsAdd::XmsgImHlrContractsAdd()
{

}

void XmsgImHlrContractsAdd::handle(shared_ptr<XmsgNeUsr> nu, SptrUsr usr, SptrClient client, SptrXitp trans, shared_ptr<XmsgImHlrContractsAddReq> req)
{
	if (req->cgt().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title format error, can not be null");
		return;
	}
	SptrCgt ctp = ChannelGlobalTitle::parse(req->cgt());
	if (ctp == nullptr)
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title format error");
		return;
	}
	if (!ctp->isUsr()) 
	{
		trans->endDesc(RET_UNSUPPORTED, "group is unsupported, channel global title: %s", req->cgt().c_str());
		return;
	}
	if (usr->dat->cgt->isSame(ctp)) 
	{
		trans->endDesc(RET_FORBIDDEN, "can not add yourself to contracts");
		return;
	}
	if (usr->contracts->getCtp(req->cgt()) != nullptr) 
	{
		trans->endDesc(RET_DUPLICATE_OPER, "contract person already existed");
		return;
	}
	if (ctp->isSameHlr(XmsgImHlrCfg::instance()->cgt))
	{
		auto ctpUsr = XmsgImUsrMgr::instance()->findXmsgImUsr(req->cgt());
		if (ctpUsr == nullptr) 
		{
			trans->endDesc(RET_FORBIDDEN, "can not found contract person for channel global title: %s", req->cgt().c_str());
			return;
		}
		ctpUsr->future([usr, trans, req, ctpUsr]
		{
			if (ctpUsr->contracts->getBlackList(usr->dat->cgt->toString()) != nullptr) 
			{
				trans->endDesc(RET_FORBIDDEN, "your are in target user`s black list");
				return;
			}
			XmsgImHlrDb::instance()->future([usr, trans, req, ctpUsr] 
					{
						XmsgImHlrContractsAdd::handleOnDbThread(usr, trans, req, ctpUsr);
					});
		});
		return;
	}
}

void XmsgImHlrContractsAdd::handleOnDbThread(SptrUsr usr, SptrXitp trans, shared_ptr<XmsgImHlrContractsAddReq> req, SptrUsr ctp)
{
	XmsgImHlrContractsAddNotice notice;
	notice.set_cgt(usr->dat->cgt->toString());
	notice.set_gts(Xsc::clock);
	shared_ptr<XmsgImHlrUsrEventColl> coll(new XmsgImHlrUsrEventColl());
	coll->cgt = ctp->dat->cgt;
	coll->isRead = false;
	coll->ent = XmsgUsrEventNoticeType::TERMINAL_ALL;
	coll->ver = ctp->evn->nextVer(); 
	coll->msg = XmsgImHlrContractsAddNotice::descriptor()->name();
	coll->dat = notice.SerializeAsString();
	coll->gts = notice.gts();
	coll->uts = coll->gts;
	coll->ets = coll->gts + XmsgImHlrCfg::instance()->cfgPb->contracts().usreventexpired() * DateMisc::sec;
	if (!XmsgImHlrUsrEventCollOper::instance()->insert(coll))
	{
		trans->endDesc(RET_EXCEPTION, "may be database exception");
		LOG_ERROR("insert usr event in to database failed, usr: %s, req: %s", usr->dat->cgt->toString().c_str(), req->ShortDebugString().c_str())
		return;
	}
	LOG_DEBUG("insert usr event in to database successful, usr: %s, req: %s", usr->dat->cgt->toString().c_str(), req->ShortDebugString().c_str())
	shared_ptr<XmsgImHlrContractsAddRsp> rsp(new XmsgImHlrContractsAddRsp());
	rsp->set_ver(coll->ver);
	trans->end(rsp);
	ctp->future([ctp, coll]
	{
		ctp->evn->onEvent(ctp, coll);
	});
}

XmsgImHlrContractsAdd::~XmsgImHlrContractsAdd()
{

}

