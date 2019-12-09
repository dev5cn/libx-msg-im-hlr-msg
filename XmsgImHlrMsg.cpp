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

#include "XmsgImHlrMsg.h"
#include "mgr/XmsgImMgrHlrSendSysNotice.h"
#include "mgr/XmsgImMgrNeNetLoad.h"
#include "mgr/XmsgImMgrNeXscServerQuery.h"
#include "mgr/XmsgImMgrNeXscWorkerCount.h"
#include "msg/XmsgImHlrAttachSimple.h"
#include "msg/XmsgImHlrContractsAdd.h"
#include "msg/XmsgImHlrContractsAddDirectly.h"
#include "msg/XmsgImHlrContractsAddReply.h"
#include "msg/XmsgImHlrContractsDel.h"
#include "msg/XmsgImHlrContractsSearch.h"
#include "msg/XmsgImHlrContractsUpdateInfo.h"
#include "msg/XmsgImHlrDetachSimple.h"
#include "msg/XmsgImHlrEventSysSub.h"
#include "msg/XmsgImHlrEventSysRead.h"
#include "msg/XmsgImHlrEventUsrRead.h"
#include "msg/XmsgImHlrEventUsrSub.h"
#include "msg/XmsgImHlrOtherUsrInfoQuery.h"
#include "msg/XmsgImHlrUsrInfoQuery.h"
#include "msg/XmsgImHlrUsrInfoUpdate.h"
#include "ne/XmsgApClientLost.h"
#include "ne/XmsgImAuthOrgRegiste.h"
#include "ne/XmsgImAuthOrgUpdateAccountInfo.h"
#include "ne/XmsgImHlrChannelStatusSub.h"
#include "ne/XmsgImHlrUsrAuthInfoQuery.h"
#include "ne/XmsgNeAuth.h"

XmsgImHlrMsg::XmsgImHlrMsg()
{

}

void XmsgImHlrMsg::init(shared_ptr<XmsgImN2HMsgMgr> priMsgMgr)
{
	X_MSG_H2N_PRPC_BEFOR_AUTH(XmsgAp, XmsgImHlrAttachSimpleReq, XmsgImHlrAttachSimpleRsp, XmsgImHlrAttachSimple::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrDetachSimpleReq, XmsgImHlrDetachSimpleRsp, XmsgImHlrDetachSimple::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrOtherUsrInfoQueryReq, XmsgImHlrOtherUsrInfoQueryRsp, XmsgImHlrOtherUsrInfoQuery::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrUsrInfoQueryReq, XmsgImHlrUsrInfoQueryRsp, XmsgImHlrUsrInfoQuery::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrUsrInfoUpdateReq, XmsgImHlrUsrInfoUpdateRsp, XmsgImHlrUsrInfoUpdate::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrContractsAddReq, XmsgImHlrContractsAddRsp, XmsgImHlrContractsAdd::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrContractsAddDirectlyReq, XmsgImHlrContractsAddDirectlyRsp, XmsgImHlrContractsAddDirectly::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrContractsAddReplyReq, XmsgImHlrContractsAddReplyRsp, XmsgImHlrContractsAddReply::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrContractsDelReq, XmsgImHlrContractsDelRsp, XmsgImHlrContractsDel::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrContractsSearchReq, XmsgImHlrContractsSearchRsp, XmsgImHlrContractsSearch::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrContractsUpdateInfoReq, XmsgImHlrContractsUpdateInfoRsp, XmsgImHlrContractsUpdateInfo::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrEventSysSubReq, XmsgImHlrEventSysSubRsp, XmsgImHlrEventSysSub::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrEventUsrSubReq, XmsgImHlrEventUsrSubRsp, XmsgImHlrEventUsrSub::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrEventSysReadReq, XmsgImHlrEventSysReadRsp, XmsgImHlrEventSysRead::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH(XmsgAp, XmsgImHlrEventUsrReadReq, XmsgImHlrEventUsrReadRsp, XmsgImHlrEventUsrRead::handle)
	X_MSG_H2N_PRPC_AFTER_AUTH_UNI(XmsgAp, XmsgApClientLostNotice, XmsgApClientLost::handle)
	X_MSG_N2H_PRPC_BEFOR_AUTH(priMsgMgr, XmsgNeAuthReq, XmsgNeAuthRsp, XmsgNeAuth::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImAuthOrgRegisteReq, XmsgImAuthOrgRegisteRsp, XmsgImAuthOrgRegiste::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImAuthOrgUpdateAccountInfoReq, XmsgImAuthOrgUpdateAccountInfoRsp, XmsgImAuthOrgUpdateAccountInfo::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImHlrChannelStatusSubReq, XmsgImHlrChannelStatusSubRsp, XmsgImHlrChannelStatusSub::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImHlrUsrAuthInfoQueryReq, XmsgImHlrUsrAuthInfoQueryRsp, XmsgImHlrUsrAuthInfoQuery::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrHlrSendSysNoticeReq, XmsgImMgrHlrSendSysNoticeRsp, XmsgImMgrHlrSendSysNotice::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeNetLoadReq, XmsgImMgrNeNetLoadRsp, XmsgImMgrNeNetLoad::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeXscServerQueryReq, XmsgImMgrNeXscServerQueryRsp, XmsgImMgrNeXscServerQuery::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeXscWorkerCountReq, XmsgImMgrNeXscWorkerCountRsp, XmsgImMgrNeXscWorkerCount::handle)
}

XmsgImHlrMsg::~XmsgImHlrMsg()
{

}

