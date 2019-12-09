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

#ifndef MSG_XMSGIMHLRATTACHSIMPLE_H_
#define MSG_XMSGIMHLRATTACHSIMPLE_H_

#include <libx-msg-im-hlr-core.h>

class XmsgImHlrAttachSimple
{
public:
	static void handle(shared_ptr<XmsgNeUsr> nu , const string& ccid, SptrXitp trans, shared_ptr<XmsgImHlrAttachSimpleReq> req); 
private:
	static void handleXmsgImAuthRsp(shared_ptr<XmsgNeUsr> nu, shared_ptr<XmsgImHlrAttachSimpleReq> req, const string& ccid, SptrXitp trans, SptrXiti itrans, shared_ptr<XmsgImUsr> usr); 
	static void attach4local(shared_ptr<XmsgAp> ap, const string& ccid, SptrXitp trans, shared_ptr<XmsgImHlrAttachSimpleReq> req, shared_ptr<XmsgImUsr> usr, shared_ptr<XmsgImAuthClientAttachSimpleRsp> rsp); 
private:
	static void kick(SptrXitp trans, shared_ptr<XmsgImUsr> usr, shared_ptr<XmsgImAuthClientAttachSimpleRsp> rsp); 
	static void kick4local(SptrXitp trans, shared_ptr<XmsgImUsr> usr, shared_ptr<XmsgImAuthClientAttachSimpleRsp> rsp, SptrClient old); 
	static void kickByLocalXmsgAp(SptrXitp trans, SptrXiti itrans, SptrClient old); 
	XmsgImHlrAttachSimple();
	virtual ~XmsgImHlrAttachSimple();
};

#endif 
