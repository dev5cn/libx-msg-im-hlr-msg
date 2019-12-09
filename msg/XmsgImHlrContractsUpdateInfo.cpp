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
#include "XmsgImHlrContractsUpdateInfo.h"

XmsgImHlrContractsUpdateInfo::XmsgImHlrContractsUpdateInfo()
{

}

void XmsgImHlrContractsUpdateInfo::handle(shared_ptr<XmsgNeUsr> nu, SptrUsr usr, SptrClient client, SptrXitp trans, shared_ptr<XmsgImHlrContractsUpdateInfoReq> req)
{
	if (req->cgt().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title can not be null");
		return;
	}
	SptrCgt cgt = ChannelGlobalTitle::parse(req->cgt());
	if (cgt == nullptr)
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title: %s", req->cgt().c_str());
		return;
	}
	if (req->upsert().empty() && req->remove().empty())
	{
		trans->endDesc(RET_FORBIDDEN, "update what?");
		return;
	}
	auto coll = usr->contracts->getCtp(req->cgt());
	if (coll == nullptr)
	{
		trans->endDesc(RET_NOT_FOUND, "can not found contract person");
		return;
	}
	shared_ptr<XmsgKv> info(new XmsgKv());
	*(info->mutable_kv()) = coll->info->kv(); 
	XmsgMisc::updateKv(req->upsert(), req->remove(), *(info->mutable_kv())); 
	XmsgImHlrDb::instance()->future([usr, trans, req, coll, info]
	{
		XmsgImHlrContractsUpdateInfo::handleOnDbThread(usr, trans, req, coll, info);
	});
}

void XmsgImHlrContractsUpdateInfo::handleOnDbThread(SptrUsr usr, SptrXitp trans, shared_ptr<XmsgImHlrContractsUpdateInfoReq> req, shared_ptr<XmsgImHlrContractsColl> coll, shared_ptr<XmsgKv> info)
{
	shared_ptr<XmsgImHlrContractsColl> c(new XmsgImHlrContractsColl());
	c->cgt = coll->cgt;
	c->ctp = coll->ctp;
	c->info = info;
	c->gts = coll->gts;
	c->uts = Xsc::clock;
	if (!XmsgImHlrContractsCollOper::instance()->update(c))
	{
		LOG_ERROR("update contract person failed, may be database exception, c: %s", c->toString().c_str())
		trans->endDesc(RET_EXCEPTION, "update contract person failed, may be database exception");
		return;
	}
	LOG_ERROR("update contract person successful, c: %s", c->toString().c_str())
	trans->success(); 
	usr->future([coll, c] 
	{
		coll->info = c->info;
		coll->uts = c->uts;
	});
}

XmsgImHlrContractsUpdateInfo::~XmsgImHlrContractsUpdateInfo()
{

}

