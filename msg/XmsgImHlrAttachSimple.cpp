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

#include "XmsgImHlrAttachSimple.h"

XmsgImHlrAttachSimple::XmsgImHlrAttachSimple()
{

}

void XmsgImHlrAttachSimple::handle(shared_ptr<XmsgNeUsr> nu , const string& ccid, SptrXitp trans, shared_ptr<XmsgImHlrAttachSimpleReq> req)
{
	shared_ptr<XmsgImUsr> usr = XmsgImUsrMgr::instance()->findXmsgImUsr(req->cgt());
	if (usr == nullptr)
	{
		LOG_ERROR("can not found x-msg-im-usr for channel global title, req: %s", req->ShortDebugString().c_str())
		trans->end(RET_FORBIDDEN);
		return;
	}
	shared_ptr<XmsgNeUsr> auth = XmsgNeMgr::instance()->getAuth(); 
	if (auth == nullptr)
	{
		LOG_ERROR("can not allocate x-msg-im-auth, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_EXCEPTION, "can not allocate x-msg-im-auth");
		return;
	}
	shared_ptr<XmsgImAuthClientAttachSimpleReq> r(new XmsgImAuthClientAttachSimpleReq());
	r->set_cgt(req->cgt());
	r->set_token(req->token());
	r->set_salt(req->salt());
	r->set_sign(req->sign());
	r->set_alg(req->alg());
	*(r->mutable_ext()) = req->ext();
	XmsgImChannel::cast(auth->channel)->begin(r, [nu, req, ccid, trans, usr](SptrXiti itrans) 
	{
		XmsgImHlrAttachSimple::handleXmsgImAuthRsp(nu, req, ccid, trans, itrans, usr);
	}, nullptr, trans);
}

void XmsgImHlrAttachSimple::handleXmsgImAuthRsp(shared_ptr<XmsgNeUsr> nu, shared_ptr<XmsgImHlrAttachSimpleReq> req, const string& ccid, SptrXitp trans, SptrXiti itrans, shared_ptr<XmsgImUsr> usr)
{
	if (itrans->ret != RET_SUCCESS) 
	{
		LOG_DEBUG("attach failed, req: %s, ccid: %s, ret: %04X, desc: %s", req->ShortDebugString().c_str(), ccid.c_str(), itrans->ret, itrans->desc.c_str())
		trans->end(itrans->ret, itrans->desc, itrans->endMsg);
		return;
	}
	shared_ptr<XmsgImAuthClientAttachSimpleRsp> rsp = static_pointer_cast<XmsgImAuthClientAttachSimpleRsp>(itrans->endMsg);
	usr->future([nu, ccid, trans, req, usr, rsp]
	{
		shared_ptr<XmsgNe> ne = static_pointer_cast<XmsgNe>(nu->channel);
		XmsgImHlrAttachSimple::attach4local(static_pointer_cast<XmsgAp>(ne), ccid, trans, req, usr, rsp);
	});
}

void XmsgImHlrAttachSimple::attach4local(shared_ptr<XmsgAp> ap, const string& ccid, SptrXitp trans, shared_ptr<XmsgImHlrAttachSimpleReq> req, shared_ptr<XmsgImUsr> usr, shared_ptr<XmsgImAuthClientAttachSimpleRsp> rsp)
{
	XmsgImHlrAttachSimple::kick(trans, usr, rsp);
	shared_ptr<XmsgImClient> client(new XmsgImClient(usr, rsp->info().plat(), rsp->info().did(), ccid, ap));
	client->evnEstb(usr, trans); 
	XmsgImUsrMgr::instance()->addXmsgImClient(client); 
	usr->addClient(client); 
	shared_ptr<XmsgImHlrAttachSimpleRsp> r(new XmsgImHlrAttachSimpleRsp());
	trans->addOob(XSC_TAG_PLATFORM, rsp->info().plat());
	trans->addOob(XSC_TAG_DEVICE_ID, rsp->info().did());
	trans->end(r);
	LOG_DEBUG("have x-msg-im-client attach to x-msg-im-hlr successful, client: %s, req: %s, rsp: %s", client->toString().c_str(), req->ShortDebugString().c_str(), r->ShortDebugString().c_str())
}

void XmsgImHlrAttachSimple::kick(SptrXitp trans, shared_ptr<XmsgImUsr> usr, shared_ptr<XmsgImAuthClientAttachSimpleRsp> rsp)
{
	if (XmsgImHlrCfg::instance()->cfgPb->misc().platcompetemode() != XmsgImHlrPlatCompeteMode::X_MSG_IM_HLR_PLAT_COMPETE_MODE_TYPE_ONLY) 
	{
		LOG_FAULT("unsupported platform compete mode: %02X", XmsgImHlrCfg::instance()->cfgPb->misc().platcompetemode())
		return;
	}
	SptrClient old = usr->removeClientByType(XmsgMisc::getPlatType(rsp->info().plat()));
	while (old != nullptr)
	{
		XmsgImHlrAttachSimple::kick4local(trans, usr, rsp, old);
		old = usr->removeClientByType(XmsgMisc::getPlatType(rsp->info().plat()));
		if (old != nullptr)
		{
			LOG_FAULT("it`s a bug, have a more than one old x-msg-im-client online, old: %s", old->toString().c_str())
		}
	}
}

void XmsgImHlrAttachSimple::kick4local(SptrXitp trans, shared_ptr<XmsgImUsr> usr, shared_ptr<XmsgImAuthClientAttachSimpleRsp> rsp, SptrClient old)
{
	auto nu = old->apLocal->usr.lock();
	if (nu == nullptr) 
		return;
	LOG_DEBUG("have an other client attached on x-msg-ap, we will kick it: %s", old->toString().c_str())
	SptrOob oob(new list<pair<uchar, string>>());
	oob->push_back(make_pair<>(XSC_TAG_UID, old->ccid));
	shared_ptr<XmsgImHlrOtherClientAttachReq> req(new XmsgImHlrOtherClientAttachReq()); 
	req->mutable_dev()->CopyFrom(rsp->info());
	XmsgImChannel::cast(old->apLocal)->begin(req, [trans, old](SptrXiti itrans) 
	{
		XmsgImHlrAttachSimple::kickByLocalXmsgAp(trans, itrans, old);
	}, oob, trans);
}

void XmsgImHlrAttachSimple::kickByLocalXmsgAp(SptrXitp trans, SptrXiti itrans, SptrClient old)
{
	auto nu = old->apLocal->usr.lock();
	if (nu == nullptr) 
		return;
	LOG_DEBUG("got a x-msg-im-client kick response, ret: %04X, desc: %s, rsp: %s", itrans->ret, itrans->desc.c_str(), itrans->endMsg == nullptr ? "null" : itrans->endMsg->ShortDebugString().c_str())
	SptrOob oob(new list<pair<uchar, string>>());
	oob->push_back(make_pair<>(XSC_TAG_INTERCEPT, "enable"));
	shared_ptr<XmsgApClientKickReq> kcr(new XmsgApClientKickReq()); 
	kcr->set_ccid(old->ccid);
	XmsgImChannel::cast(old->apLocal)->begin(kcr, [kcr](SptrXiti itransx) 
	{
		LOG_DEBUG("got a x-msg-im-client kick response from x-msg-ap, ret: %04X, desc: %s, rsp: %s", itransx->ret, itransx->desc.c_str(), itransx->endMsg == nullptr ? "null" : itransx->endMsg->ShortDebugString().c_str())
	}, oob);
}

XmsgImHlrAttachSimple::~XmsgImHlrAttachSimple()
{

}

