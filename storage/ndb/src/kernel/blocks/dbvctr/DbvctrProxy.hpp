/* Copyright (c) 2025, IP-Solutions AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef NDB_DBVCTR_PROXY_HPP
#define NDB_DBVCTR_PROXY_HPP

#include <LocalProxy.hpp>
#include <signaldata/AlterIndxImpl.hpp>
#include <signaldata/DropTab.hpp>
#include <signaldata/IndexStatSignal.hpp>

#define JAM_FILE_ID 558

class DbvctrProxy : public LocalProxy {
 public:
  DbvctrProxy(Block_context &ctx);
  ~DbvctrProxy() override;
  BLOCK_DEFINES(DbvctrProxy);

 protected:
  SimulatedBlock *newWorker(Uint32 instanceNo) override;

  // GSN_ALTER_INDX_IMPL_REQ
  struct Ss_ALTER_INDX_IMPL_REQ : SsParallel {
    AlterIndxImplReq m_req;
    Ss_ALTER_INDX_IMPL_REQ() {
      m_sendREQ = (SsFUNCREQ)&DbvctrProxy::sendALTER_INDX_IMPL_REQ;
      m_sendCONF = (SsFUNCREP)&DbvctrProxy::sendALTER_INDX_IMPL_CONF;
    }
    enum { poolSize = 1 };
    static SsPool<Ss_ALTER_INDX_IMPL_REQ> &pool(LocalProxy *proxy) {
      return ((DbvctrProxy *)proxy)->c_ss_ALTER_INDX_IMPL_REQ;
    }
  };
  SsPool<Ss_ALTER_INDX_IMPL_REQ> c_ss_ALTER_INDX_IMPL_REQ;
  void execALTER_INDX_IMPL_REQ(Signal *);
  void sendALTER_INDX_IMPL_REQ(Signal *, Uint32 ssId, SectionHandle *);
  void execALTER_INDX_IMPL_CONF(Signal *);
  void execALTER_INDX_IMPL_REF(Signal *);
  void sendALTER_INDX_IMPL_CONF(Signal *, Uint32 ssId);

  // GSN_INDEX_STAT_IMPL_REQ
  struct Ss_INDEX_STAT_IMPL_REQ : SsParallel {
    IndexStatImplReq m_req;
    Ss_INDEX_STAT_IMPL_REQ() {
      m_sendREQ = (SsFUNCREQ)&DbvctrProxy::sendINDEX_STAT_IMPL_REQ;
      m_sendCONF = (SsFUNCREP)&DbvctrProxy::sendINDEX_STAT_IMPL_CONF;
    }
    enum { poolSize = 1 };
    static SsPool<Ss_INDEX_STAT_IMPL_REQ> &pool(LocalProxy *proxy) {
      return ((DbvctrProxy *)proxy)->c_ss_INDEX_STAT_IMPL_REQ;
    }
  };
  SsPool<Ss_INDEX_STAT_IMPL_REQ> c_ss_INDEX_STAT_IMPL_REQ;
  void execINDEX_STAT_IMPL_REQ(Signal *);
  void sendINDEX_STAT_IMPL_REQ(Signal *, Uint32 ssId, SectionHandle *);
  void execINDEX_STAT_IMPL_CONF(Signal *);
  void execINDEX_STAT_IMPL_REF(Signal *);
  void sendINDEX_STAT_IMPL_CONF(Signal *, Uint32 ssId);

  // GSN_INDEX_STAT_REP
  void execINDEX_STAT_REP(Signal *);
};

#undef JAM_FILE_ID

#endif
