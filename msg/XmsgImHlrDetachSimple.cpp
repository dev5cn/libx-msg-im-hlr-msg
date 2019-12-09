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

#include "XmsgImHlrDetachSimple.h"

XmsgImHlrDetachSimple::XmsgImHlrDetachSimple()
{

}

void XmsgImHlrDetachSimple::handle(shared_ptr<XmsgNeUsr> nu, SptrUsr usr, SptrClient client, SptrXitp trans, shared_ptr<XmsgImHlrDetachSimpleReq> req)
{
	shared_ptr<XmsgImHlrDetachSimpleRsp> rsp(new XmsgImHlrDetachSimpleRsp());
	XmsgMisc::insertKv(rsp->mutable_ext(), "accept", "true");
	trans->end(rsp);
	LOG_DEBUG("have a x-msg-im-client detach from x-msg-im-hlr, client: %s", client->toString().c_str())
	shared_ptr<XmsgApClientKickReq> kcr(new XmsgApClientKickReq());
	kcr->set_ccid(client->ccid);
	SptrOob oob(new list<pair<uchar, string>>());
	oob->push_back(make_pair<>(XSC_TAG_INTERCEPT, "enable"));
	XmsgImChannel::cast(nu->channel)->begin(kcr, [kcr](SptrXiti itrans) 
	{
		LOG_DEBUG("got a x-msg-im-client kick response from x-msg-ap, ret: %04X, desc: %s, rsp: %s", itrans->ret, itrans->desc.c_str(), itrans->endMsg == nullptr ? "null" : itrans->endMsg->ShortDebugString().c_str())
	}, oob, trans);
	shared_ptr<XmsgApClientLostNotice> notice(new XmsgApClientLostNotice());
	notice->set_cgt(usr->dat->cgt->toString());
	notice->set_plat(client->plat);
	notice->set_did(client->did);
	notice->set_ccid(client->ccid);
	client->evnDisc(usr, notice, trans);
}

XmsgImHlrDetachSimple::~XmsgImHlrDetachSimple()
{

}

