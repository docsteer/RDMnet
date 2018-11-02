/******************************************************************************
************************* IMPORTANT NOTE -- READ ME!!! ************************
*******************************************************************************
* THIS SOFTWARE IMPLEMENTS A **DRAFT** STANDARD, BSR E1.33 REV. 63. UNDER NO
* CIRCUMSTANCES SHOULD THIS SOFTWARE BE USED FOR ANY PRODUCT AVAILABLE FOR
* GENERAL SALE TO THE PUBLIC. DUE TO THE INEVITABLE CHANGE OF DRAFT PROTOCOL
* VALUES AND BEHAVIORAL REQUIREMENTS, PRODUCTS USING THIS SOFTWARE WILL **NOT**
* BE INTEROPERABLE WITH PRODUCTS IMPLEMENTING THE FINAL RATIFIED STANDARD.
*******************************************************************************
* Copyright 2018 ETC Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************
* This file is a part of RDMnet. For more information, go to:
* https://github.com/ETCLabs/RDMnet
******************************************************************************/

#include "broker/util.h"
#include <cassert>

/* Suppress strncpy() warning on Windows/MSVC. */
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

RptHeader SwapHeaderData(const RptHeader &source)
{
  RptHeader swapped_header;

  swapped_header.seqnum = source.seqnum;
  swapped_header.dest_endpoint_id = source.source_endpoint_id;
  swapped_header.dest_uid = source.source_uid;
  swapped_header.source_endpoint_id = source.dest_endpoint_id;
  swapped_header.source_uid = source.dest_uid;
  return swapped_header;
}

static void BrokerLogCallback(void *context, const char * /*syslog_str*/, const char *human_str,
                              const char * /*raw_str*/)
{
  assert(human_str);
  BrokerLog *bl = static_cast<BrokerLog *>(context);
  if (bl)
    bl->LogFromCallback(human_str);
}

static void BrokerTimeCallback(void *context, LwpaLogTimeParams *time)
{
  BrokerLog *bl = static_cast<BrokerLog *>(context);
  if (bl)
    bl->GetTimeFromCallback(time);
}

static void log_thread_fn(void *arg)
{
  BrokerLog *bl = static_cast<BrokerLog *>(arg);
  if (bl)
    bl->LogThreadRun();
}

BrokerLog::BrokerLog() : keep_running_(false)
{
  lwpa_signal_create(&signal_);
  lwpa_mutex_create(&lock_);
}

BrokerLog::~BrokerLog()
{
  StopThread();
  lwpa_mutex_destroy(&lock_);
  lwpa_signal_destroy(&signal_);
}

void BrokerLog::InitializeLogParams(int log_mask)
{
  log_params_.action = kLwpaLogCreateHumanReadableLog;
  log_params_.log_fn = BrokerLogCallback;
  log_params_.log_mask = log_mask;
  log_params_.time_fn = BrokerTimeCallback;
  log_params_.context = this;

  lwpa_validate_log_params(&log_params_);
}

bool BrokerLog::StartThread()
{
  keep_running_ = true;
  LwpaThreadParams tparams = {LWPA_THREAD_DEFAULT_PRIORITY, LWPA_THREAD_DEFAULT_STACK, "BrokerLogThread", NULL};
  return lwpa_thread_create(&thread_, &tparams, log_thread_fn, this);
}

void BrokerLog::StopThread()
{
  if (keep_running_)
  {
    keep_running_ = false;
    lwpa_signal_post(&signal_);
    lwpa_thread_stop(&thread_, 10000);
  }
}

void BrokerLog::Log(int pri, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  lwpa_vlog(&log_params_, pri, format, args);
  va_end(args);
}

void BrokerLog::LogFromCallback(const std::string &str)
{
  {
    BrokerMutexGuard guard(lock_);
    msg_q_.push(str);
  }
  lwpa_signal_post(&signal_);
}

void BrokerLog::LogThreadRun()
{
  while (keep_running_)
  {
    lwpa_signal_wait(&signal_, LWPA_WAIT_FOREVER);
    if (keep_running_)
    {
      std::vector<std::string> to_log;

      {
        BrokerMutexGuard guard(lock_);
        to_log.reserve(msg_q_.size());
        while (!msg_q_.empty())
        {
          to_log.push_back(msg_q_.front());
          msg_q_.pop();
        }
      }
      for (auto log_msg : to_log)
      {
        OutputLogMsg(log_msg);
      }
    }
  }
}
