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
#include "XmsgImHlrContractsDel.h"

XmsgImHlrContractsDel::XmsgImHlrContractsDel()
{

}

void XmsgImHlrContractsDel::handle(shared_ptr<XmsgNeUsr> nu, SptrUsr usr, SptrClient client, SptrXitp trans, shared_ptr<XmsgImHlrContractsDelReq> req)
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
	if (usr->contracts->getCtp(req->cgt()) == nullptr)
	{
		trans->endDesc(RET_NOT_FOUND, "can not found contract person");
		return;
	}
	XmsgImHlrDb::instance()->future([usr, trans, req, ctp]
	{
		XmsgImHlrContractsDel::handleOnDbThread(usr, trans, req, ctp);
	});
}

void XmsgImHlrContractsDel::handleOnDbThread(SptrUsr usr, SptrXitp trans, shared_ptr<XmsgImHlrContractsDelReq> req, SptrCgt ctp)
{
	if (!XmsgImHlrContractsCollOper::instance()->del(usr->dat->cgt, ctp))
	{
		LOG_ERROR("delete contract person failed, may be database exception, usr: %s, req: %s", usr->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_EXCEPTION, "delete contract person failed, may be database exception");
		return;
	}
	LOG_DEBUG("delete contract person successful, usr: %s, req: %s", usr->toString().c_str(), req->ShortDebugString().c_str())
	trans->success();
	usr->future([usr, ctp]
	{
		usr->contracts->delCtp(ctp); 
	});
	if (XmsgMisc::getBool(req->ext(), "blackList"))
	{
		shared_ptr<XmsgImHlrContractsBlackListColl> coll(new XmsgImHlrContractsBlackListColl());
		coll->cgt = usr->dat->cgt;
		coll->ctp = ctp;
		coll->info.reset(new XmsgKv());
		coll->gts = Xsc::clock;
		if (!XmsgImHlrContractsBlackListCollOper::instance()->insert(coll))
		{
			LOG_ERROR("add contract person to black-list failed, may be database exception, coll: %s", coll->toString().c_str())
		} else
		{
			LOG_DEBUG("add contract person to black-list successful, coll: %s", coll->toString().c_str())
			usr->future([usr, coll]
			{
				usr->contracts->addBlackList(coll);
			});
		}
	}
	if (!XmsgMisc::getBool(req->ext(), "deleteFromOtherSide"))
		return;
	XmsgImHlrContractsDel::notice2ctpLocal(usr, trans, ctp);
}

void XmsgImHlrContractsDel::notice2ctpLocal(SptrUsr usr, SptrXitp trans, SptrCgt ctp)
{
	if (!XmsgImHlrContractsCollOper::instance()->del(ctp, usr->dat->cgt)) 
	{
		LOG_ERROR("delete contract person failed, may be database exception, usr: %s, ctp: %s", ctp->toString().c_str(), usr->dat->cgt->toString().c_str())
		return;
	}
	LOG_DEBUG("delete contract person successful, usr: %s, ctp: %s", ctp->toString().c_str(), usr->dat->cgt->toString().c_str())
}

XmsgImHlrContractsDel::~XmsgImHlrContractsDel()
{

}

