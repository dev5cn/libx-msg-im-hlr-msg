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
#include "XmsgImHlrContractsAddDirectly.h"

XmsgImHlrContractsAddDirectly::XmsgImHlrContractsAddDirectly()
{

}

void XmsgImHlrContractsAddDirectly::handle(shared_ptr<XmsgNeUsr> nu, SptrUsr usr, SptrClient client, SptrXitp trans, shared_ptr<XmsgImHlrContractsAddDirectlyReq> req)
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
	auto ctpUsr = XmsgImUsrMgr::instance()->findXmsgImUsr(req->cgt());
	if (ctpUsr == nullptr) 
	{
		trans->endDesc(RET_FORBIDDEN, "can not found contract person for channel global title: %s", req->cgt().c_str());
		return;
	}
	ctpUsr->future([usr, trans, req, ctp, ctpUsr]
	{
		if (ctpUsr->contracts->getBlackList(usr->dat->cgt->toString()) != nullptr) 
		{
			trans->endDesc(RET_FORBIDDEN, "your are in target user`s black list");
			return;
		}
		XmsgImHlrDb::instance()->future([usr, trans, req, ctp] 
				{
					XmsgImHlrContractsAddDirectly::handleOnDbThread(usr, trans, req, ctp);
				});
	});
}

void XmsgImHlrContractsAddDirectly::handleOnDbThread(shared_ptr<XmsgImUsr> usr, SptrXitp trans, shared_ptr<XmsgImHlrContractsAddDirectlyReq> req, SptrCgt ctp)
{
	shared_ptr<XmsgImHlrContractsColl> coll(new XmsgImHlrContractsColl());
	coll->cgt = usr->dat->cgt;
	coll->ctp = ctp;
	coll->info.reset(new XmsgKv());
	*(coll->info->mutable_kv()) = req->info(); 
	coll->gts = Xsc::clock;
	coll->uts = coll->gts;
	if (!XmsgImHlrContractsCollOper::instance()->insert(coll))
	{
		LOG_ERROR("add to contracts failed, may be database exception, usr: %s, req: %s", usr->dat->cgt->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_EXCEPTION, "may be database exception");
		return;
	}
	LOG_DEBUG("add to contracts successful, usr: %s, req: %s", usr->dat->cgt->toString().c_str(), req->ShortDebugString().c_str())
	shared_ptr<XmsgImHlrContractsAddDirectlyRsp> rsp(new XmsgImHlrContractsAddDirectlyRsp());
	XmsgMisc::insertKv(rsp->mutable_ext(), "accept", "true");
	trans->end(rsp);
	usr->future([usr, coll]
	{
		usr->contracts->addCtp(coll); 
	});
}

XmsgImHlrContractsAddDirectly::~XmsgImHlrContractsAddDirectly()
{

}

