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
#include "XmsgImHlrContractsSearch.h"

XmsgImHlrContractsSearch::XmsgImHlrContractsSearch()
{

}

void XmsgImHlrContractsSearch::handle(shared_ptr<XmsgNeUsr> nu, SptrUsr usr, SptrClient client, SptrXitp trans, shared_ptr<XmsgImHlrContractsSearchReq> req)
{
	if (req->keyword().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "keyword can not be null");
		return;
	}
	list<shared_ptr<XmsgImUsr>> lis;
	XmsgImUsrMgr::instance()->search(req->keyword(), lis);
	if (lis.empty())
	{
		trans->end(RET_NO_RECORD);
		return;
	}
	shared_ptr<XmsgImHlrContractsSearchRsp> rsp(new XmsgImHlrContractsSearchRsp());
	for (auto& it : lis)
	{
		auto rst = rsp->add_rst();
		rst->set_cgt(it->dat->cgt->toString());
		rst->set_name(it->dat->pub->name());
		*(rst->mutable_info()) = it->dat->pub->info();
	}
	trans->end(rsp);
}

XmsgImHlrContractsSearch::~XmsgImHlrContractsSearch()
{

}

