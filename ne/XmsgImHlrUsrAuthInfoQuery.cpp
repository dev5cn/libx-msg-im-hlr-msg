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

#include "XmsgImHlrUsrAuthInfoQuery.h"

XmsgImHlrUsrAuthInfoQuery::XmsgImHlrUsrAuthInfoQuery()
{

}

void XmsgImHlrUsrAuthInfoQuery::handle(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgImHlrUsrAuthInfoQueryReq> req)
{
	if (req->token().empty())
	{
		LOG_ERROR("token can not be null, nu: %s, req: %s", nu->toString().c_str(), req->ShortDebugString().c_str())
		trans->endDesc(RET_FORMAT_ERROR, "token can not be null");
		return;
	}
	shared_ptr<XmsgNeUsr> auth = XmsgNeMgr::instance()->getAuth(); 
	if (auth == nullptr)
	{
		LOG_ERROR("can not allocate x-msg-im-auth, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_EXCEPTION, "can not allocate x-msg-im-auth");
		return;
	}
	shared_ptr<XmsgImAuthUsrAuthInfoQueryReq> r(new XmsgImAuthUsrAuthInfoQueryReq());
	r->set_token(req->token());
	XmsgImChannel::cast(auth->channel)->begin(r, [trans](SptrXiti itrans) 
	{
		XmsgImHlrUsrAuthInfoQuery::handleXmsgImAuthRsp(trans, itrans);
	}, nullptr, trans);
}

void XmsgImHlrUsrAuthInfoQuery::handleXmsgImAuthRsp(SptrXitp trans, SptrXiti itrans)
{
	if (itrans->ret != RET_SUCCESS) 
	{
		LOG_DEBUG("query auth info from x-msg-im-auth failed, req: %s, ret: %04X, desc: %s", itrans->beginMsg->ShortDebugString().c_str(), itrans->ret, itrans->desc.c_str())
		trans->end(itrans->ret, itrans->desc, itrans->endMsg);
		return;
	}
	shared_ptr<XmsgImAuthUsrAuthInfoQueryRsp> r = static_pointer_cast<XmsgImAuthUsrAuthInfoQueryRsp>(itrans->endMsg);
	shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> rsp(new XmsgImHlrUsrAuthInfoQueryRsp());
	rsp->set_secret(r->secret());
	rsp->set_gts(r->gts());
	rsp->set_expired(r->expired());
	rsp->mutable_info()->CopyFrom(r->info());
	trans->end(rsp);
}

XmsgImHlrUsrAuthInfoQuery::~XmsgImHlrUsrAuthInfoQuery()
{

}

