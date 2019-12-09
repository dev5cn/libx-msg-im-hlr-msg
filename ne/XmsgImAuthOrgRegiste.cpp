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
#include "XmsgImAuthOrgRegiste.h"

XmsgImAuthOrgRegiste::XmsgImAuthOrgRegiste()
{

}

void XmsgImAuthOrgRegiste::handle(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgImAuthOrgRegisteReq> req)
{
	auto auth = XmsgNeMgr::instance()->getAuth();
	if (auth == nullptr)
	{
		LOG_ERROR("can not allocate x-msg-im-auth, req: %s", req->ShortDebugString().c_str())
		trans->endDesc(RET_EXCEPTION, "system exception");
		return;
	}
	XmsgImChannel::cast(auth->channel)->begin(req, [trans, req](SptrXiti itrans)
	{
		trans->end(itrans->ret, itrans->desc, itrans->endMsg); 
		if (itrans->ret != RET_SUCCESS)
		{
			LOG_ERROR("update x-msg-im-usr account info with x-msg-im-auth failed, ret: %04X, desc: %s, req: %s", itrans->ret, itrans->desc.c_str(), req->ShortDebugString().c_str())
			return;
		}
		XmsgImAuthOrgRegiste::init4hlr(trans, itrans, static_pointer_cast<XmsgImAuthOrgRegisteRsp>(itrans->endMsg));
	}, nullptr, trans);
}

void XmsgImAuthOrgRegiste::init4hlr(SptrXitp trans, SptrXiti itrans, shared_ptr<XmsgImAuthOrgRegisteRsp> rsp)
{
	SptrCgt cgt = ChannelGlobalTitle::parse(rsp->cgt());
	if (cgt == nullptr)
	{
		LOG_FAULT("it`s a bug channel global title format error, rsp: %s", rsp->ShortDebugString().c_str())
		trans->endDesc(RET_EXCEPTION, "system exception");
		return;
	}
	shared_ptr<XmsgImHlrUsrDatColl> coll(new XmsgImHlrUsrDatColl());
	coll->cgt = cgt;
	coll->ver = 1ULL; 
	coll->pri.reset(new XmsgImHlrUsrDatPri());
	coll->pri->set_enable(true);
	coll->pub.reset(new XmsgImHlrUsrDatPub());
	coll->gts = Xsc::clock;
	coll->uts = coll->gts;
	ullong sts = DateMisc::dida();
	XmsgImHlrDb::instance()->future([trans, itrans, coll, rsp, sts] 
	{
		if (!XmsgImHlrUsrDatCollOper::instance()->insert(coll))
		{
			LOG_ERROR("init usr dat in x-msg-im-hlr failed, may be database exception, elap: %dms, coll: %s", DateMisc::elapDida(sts), coll->toString().c_str())
			trans->endDesc(RET_EXCEPTION, "database exception");
			return;
		}
		LOG_DEBUG("init usr dat in x-msg-im-hlr successful, elap: %dms, coll: %s", DateMisc::elapDida(sts), coll->toString().c_str())
		shared_ptr<XmsgImUsr> usr(new XmsgImUsr(coll));
		XmsgImUsrMgr::instance()->addXmsgImUsr(usr); 
		trans->end(rsp); 
		XmsgImAuthOrgRegiste::init4group(trans, itrans, usr); 
	});
}

void XmsgImAuthOrgRegiste::init4group(SptrXitp trans, SptrXiti itrans, shared_ptr<XmsgImUsr> usr)
{
	auto group = XmsgNeMgr::instance()->getGroup();
	if (group == nullptr)
	{
		LOG_ERROR("can not allocate x-msg-im-group, usr: %s", usr->dat->toString().c_str())
		trans->endDesc(RET_EXCEPTION, "system exception");
		return;
	}
	shared_ptr<XmsgImHlrUsrInitReq> req(new XmsgImHlrUsrInitReq());
	req->set_cgt(usr->dat->cgt->toString());
	XmsgImChannel::cast(group->channel)->begin(req, [req, usr](SptrXiti xiti)
	{
		if (xiti->ret != RET_SUCCESS)
		{
			LOG_FAULT("registe x-msg-im account successful, but x-msg-im-group init usr group data failed, usr: %s, req: %s, ret: %04X, desc: %s", usr->dat->toString().c_str(), req->ShortDebugString().c_str(), xiti->ret, xiti->desc.c_str())
			return;
		}
		LOG_INFO("x-msg-im-group init usr group data successful, usr: %s, req: %s, rsp: %s", usr->dat->toString().c_str(), req->ShortDebugString().c_str(), xiti->endMsg == nullptr ? "null" : xiti->endMsg->ShortDebugString().c_str())
	}, nullptr, itrans);
}

XmsgImAuthOrgRegiste::~XmsgImAuthOrgRegiste()
{

}

