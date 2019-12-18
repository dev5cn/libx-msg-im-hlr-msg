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

#include "XmsgImHlrOtherUsrInfoQuery.h"

XmsgImHlrOtherUsrInfoQuery::XmsgImHlrOtherUsrInfoQuery()
{

}

void XmsgImHlrOtherUsrInfoQuery::handle(shared_ptr<XmsgNeUsr> nu, SptrUsr usr, SptrClient client, SptrXitp trans, shared_ptr<XmsgImHlrOtherUsrInfoQueryReq> req)
{
	if (req->cgt().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "cgt can not be null");
		return;
	}
	shared_ptr<vector<shared_ptr<XmsgImUsr>>> others(new vector<shared_ptr<XmsgImUsr>>());
	for (auto& it : req->cgt())
	{
		auto cgt = ChannelGlobalTitle::parse(it);
		if (cgt == nullptr)
		{
			trans->endDesc(RET_FORMAT_ERROR, "have some one cgt format error: %s", it.c_str());
			return;
		}
		shared_ptr<XmsgImUsr> other = XmsgImUsrMgr::instance()->findXmsgImUsr(it);
		if (other != nullptr) 
			others->push_back(other);
	}
	if (others->empty()) 
	{
		trans->end(RET_NO_RECORD);
		return;
	}
	shared_ptr<atomic_int> counter(new atomic_int(others->size()));
	shared_ptr<vector<shared_ptr<XmsgImHlrUsrDatPub>>> pubs(new vector<shared_ptr<XmsgImHlrUsrDatPub>>(others->size()));
	pubs->resize(others->size());
	for (size_t i = 0; i < others->size(); ++i)
	{
		auto other = others->at(i);
		other->future([others, other, counter, pubs, trans] 
		{
			int index = counter->fetch_sub(1) - 1;
			shared_ptr<XmsgImHlrUsrDatPub> pub(new XmsgImHlrUsrDatPub());
			pub->CopyFrom(*other->dat->pub);
			(*pubs)[index] = pub;
			if(index == 0) 
			{
				XmsgImHlrOtherUsrInfoQuery::finished(trans, others, pubs);
			}
		});
	}
}

void XmsgImHlrOtherUsrInfoQuery::finished(SptrXitp trans, shared_ptr<vector<shared_ptr<XmsgImUsr>>> others, shared_ptr<vector<shared_ptr<XmsgImHlrUsrDatPub>>> pubs)
{
	shared_ptr<XmsgImHlrOtherUsrInfoQueryRsp> rsp(new XmsgImHlrOtherUsrInfoQueryRsp());
	auto info = rsp->mutable_usrinfo();
	for (size_t i = 0; i < pubs->size(); ++i)
	{
		auto pub = pubs->at(i);
		XmsgImHlrOtherUsrInfo oui;
		*(oui.mutable_info()) = pub->info();
		info->insert(MapPair<string, XmsgImHlrOtherUsrInfo>(others->at(i)->dat->cgt->toString(), oui));
	}
	trans->end(rsp);
}

XmsgImHlrOtherUsrInfoQuery::~XmsgImHlrOtherUsrInfoQuery()
{

}

