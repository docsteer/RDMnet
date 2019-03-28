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

#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <vector>
#include <cstddef>
#include "lwpa/log.h"
#include "lwpa/thread.h"
#include "lwpa/lock.h"
#include "BrokerItem.h"
#include "SearchingStatusItem.h"
#include "PropertyValueItem.h"
#include "ControllerUtils.h"
#include "RDMnetLibWrapper.h"
#include "ControllerLog.h"

BEGIN_INCLUDE_QT_HEADERS()
#include <QStandardItemModel>
END_INCLUDE_QT_HEADERS()

#define DEVICE_LABEL_MAX_LEN 32

class BrokerConnection;

void appendRowToItem(QStandardItem *parent, QStandardItem *child);

class BrokerConnection
{
private:
  bool connected_{false};
  bool using_mdns_{false};

  rdmnet_client_scope_t conn_{RDMNET_CLIENT_SCOPE_INVALID};
  QString scope_;
  LwpaSockaddr broker_addr_;

  BrokerItem *broker_item_{nullptr};

  uint32_t sequence_{0};

  lwpa_thread_t connect_thread_;
  bool connect_in_progress_{false};

public:
  explicit BrokerConnection(const std::string &scope);
  BrokerConnection(const std::string &scope, const LwpaSockaddr &addr);
  ~BrokerConnection() {}

  // Accessors
  bool connected() const { return connected_; }
  const std::string scope() const { return scope_.toStdString(); }
  rdmnet_client_scope_t handle() const { return conn_; }

  BrokerItem *treeBrokerItem() const { return broker_item_; }

  uint32_t sequence() const { return sequence_; }

  const QString generateBrokerItemText();

  bool operator==(const BrokerConnection &other) { return scope_ == other.scope_; }

  uint32_t sequencePreIncrement() { return ++sequence_; }

  void wasDisconnected();

  void appendBrokerItemToTree(QStandardItem *invisibleRootItem, uint32_t connectionCookie);

  bool isUsingMDNS();

  LwpaSockaddr currentSockAddr();
  LwpaSockaddr staticSockAddr();
};

typedef struct RdmParamData
{
  uint8_t datalen;
  uint8_t data[RDM_MAX_PDL];
} RdmParamData;

typedef struct DefaultResponderPropertyData
{
  uint32_t endpoint_list_change_number;
  bool identifying;
  char device_label[DEVICE_LABEL_MAX_LEN + 1];
  char search_domain[E133_DOMAIN_STRING_PADDED_LENGTH];
  uint16_t tcp_unhealthy_counter;
} DefaultResponderPropertyData;

class RDMnetNetworkModel : public QStandardItemModel, public RDMnetLibNotify
{
  Q_OBJECT

public:
  std::map<rdmnet_client_scope_t, BrokerItem *> brokers_;

signals:
  void addRDMnetClients(BrokerConnection *brokerConn, const std::vector<ClientEntryData> &list);
  void removeRDMnetClients(BrokerConnection *brokerConn, const std::vector<ClientEntryData> &list);
  void newEndpointList(RDMnetClientItem *treeClientItem, const std::vector<std::pair<uint16_t, uint8_t>> &list);
  void newResponderList(EndpointItem *treeEndpointItem, const std::vector<RdmUid> &list);
  void setPropertyData(RDMnetNetworkItem *parent, unsigned short pid, const QString &name, const QVariant &value,
                       int role = Qt::DisplayRole);
  void removePropertiesInRange(RDMnetNetworkItem *parent, std::vector<class PropertyItem *> *properties,
                               unsigned short pid, int role, const QVariant &min, const QVariant &max);
  void brokerItemTextUpdated(const BrokerItem *item);
  void addPropertyEntry(RDMnetNetworkItem *parent, unsigned short pid, const QString &name, int role);
  void featureSupportChanged(const class RDMnetNetworkItem *item, SupportedDeviceFeature feature);
  void expandNewItem(const QModelIndex &index, int type);
  void identifyChanged(const RDMnetNetworkItem *item, bool identify);

private:
  ControllerLog *log_;
  RDMnetLibInterface *rdmnet_;
  uint16_t broker_create_count_ = 0;
  bool recv_thread_run_;
  lwpa_thread_t recv_thread_;

  DefaultResponderPropertyData prop_data_;

  // Protects prop_data_ and broker_connections_.
  lwpa_rwlock_t prop_lock;

  // Keeps track of scope updates of other controllers
  std::map<RdmUid, uint16_t> previous_slot_;

public slots:
  void addScopeToMonitor(std::string scope);
  void directChildrenRevealed(const QModelIndex &parentIndex);
  void addBrokerByIP(std::string scope, const LwpaSockaddr &addr);
  void addCustomLogOutputStream(LogOutputStream *stream);
  void removeCustomLogOutputStream(LogOutputStream *stream);

protected slots:
  void processAddRDMnetClients(BrokerConnection *brokerConn, const std::vector<ClientEntryData> &list);
  void processRemoveRDMnetClients(BrokerConnection *brokerConn, const std::vector<ClientEntryData> &list);
  void processNewEndpointList(RDMnetClientItem *treeClientItem, const std::vector<std::pair<uint16_t, uint8_t>> &list);
  void processNewResponderList(EndpointItem *treeEndpointItem, const std::vector<RdmUid> &list);
  void processSetPropertyData(RDMnetNetworkItem *parent, unsigned short pid, const QString &name, const QVariant &value,
                              int role);
  void processRemovePropertiesInRange(RDMnetNetworkItem *parent, std::vector<class PropertyItem *> *properties,
                                      unsigned short pid, int role, const QVariant &min, const QVariant &max);
  void processAddPropertyEntry(RDMnetNetworkItem *parent, unsigned short pid, const QString &name, int role);
  void processPropertyButtonClick(const QPersistentModelIndex &propertyIndex);
  void removeBroker(BrokerItem *brokerItem);
  void removeAllBrokers();
  void activateFeature(RDMnetNetworkItem *device, SupportedDeviceFeature feature);

public:
  void InitRDMnet();
  void ShutdownRDMnet();

  static RDMnetNetworkModel *makeRDMnetNetworkModel(RDMnetLibWrapper *library, ControllerLog *log);
  static RDMnetNetworkModel *makeTestModel();

  void searchingItemRevealed(SearchingStatusItem *searchItem);

  size_t getNumberOfCustomLogOutputStreams();

  /******* QStandardItemModel overrides *******/

  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

protected:
  // RDMnetLibNotify overrides
  virtual void Connected(rdmnet_client_scope_t scope_handle, const RdmnetClientConnectedInfo &info) override;
  virtual void NotConnected(rdmnet_client_scope_t scope_handle, const RdmnetClientNotConnectedInfo &info) override;
  virtual void ClientListUpdate(rdmnet_client_scope_t scope_handle, client_list_action_t action,
                                const ClientList &list) override;
  virtual void RdmCommandReceived(rdmnet_client_scope_t scope_handle, const RemoteRdmCommand &cmd) override;
  virtual void RdmResponseReceived(rdmnet_client_scope_t scope_handle, const RemoteRdmResponse &resp) override;
  virtual void StatusReceived(rdmnet_client_scope_t scope_handle, const RemoteRptStatus &status) override;

  /******* RDM message handling functions *******/
  // void ProcessBrokerMessage(uint16_t conn, const RdmnetMessage *msg);

  void ProcessRDMCommand(uint16_t conn, const RptHeader *header, const RdmCommand &cmd);
  // bool DefaultResponderGet(uint16_t pid, const uint8_t *param_data, uint8_t param_data_len,
  //                         RdmParamData *resp_data_list, size_t *num_responses, uint16_t *nack_reason);
  void ProcessRDMResponse(uint16_t conn, bool have_command, const RdmCommand &cmd,
                          const std::vector<RdmResponse> &response);

  // Use this with data that has identical GET_COMMAND_RESPONSE and SET_COMMAND forms.
  void ProcessRDMGetSetData(uint16_t conn, uint16_t param_id, const uint8_t *data, uint8_t datalen,
                            const RdmResponse *firstResp);

  BrokerConnection *getBrokerConnection(const std::string &scope);

  bool SendRDMCommand(const RdmCommand &cmd, BrokerItem *brokerItem);
  bool SendRDMCommand(const RdmCommand &cmd, uint16_t conn);
  void SendRDMGetResponses(const RdmUid &dest_uid, uint16_t dest_endpoint_id, uint16_t param_id,
                           const RdmParamData *resp_data_list, size_t num_responses, uint32_t seqnum,
                           uint8_t transaction_num, uint16_t conn = 0xFFFF);
  void SendNACK(uint16_t conn, const RptHeader *received_header, const RdmCommand *cmd_data, uint16_t nack_reason);
  void SendNotification(uint16_t conn, const RptHeader *received_header, const std::vector<RdmResponse> &resp_list);
  void SendNotification(uint16_t conn, const RdmUid &dest_uid, uint16_t dest_endpoint_id, uint32_t seqnum,
                        const std::vector<RdmResponse> &resp_list);
  lwpa_error_t SendNotificationOnConnection(const BrokerConnection *connection, const RptHeader &header,
                                            const std::vector<RdmResponse> &resp_list);

  /* GET/SET PROCESSING */

  bool getIdentifyDevice(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                         size_t *num_responses, uint16_t *nack_reason);
  bool getDeviceLabel(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                      size_t *num_responses, uint16_t *nack_reason);
  bool getComponentScope(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                         size_t *num_responses, uint16_t *nack_reason);
  bool getComponentScope(uint16_t slot, RdmParamData *resp_data_list, size_t *num_responses,
                         uint16_t *nack_reason = NULL);
  bool getSearchDomain(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                       size_t *num_responses, uint16_t *nack_reason);
  bool getTCPCommsStatus(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                         size_t *num_responses, uint16_t *nack_reason);
  bool getSupportedParameters(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                              size_t *num_responses, uint16_t *nack_reason);
  bool getDeviceInfo(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                     size_t *num_responses, uint16_t *nack_reason);
  bool getManufacturerLabel(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                            size_t *num_responses, uint16_t *nack_reason);
  bool getDeviceModelDescription(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                                 size_t *num_responses, uint16_t *nack_reason);
  bool getSoftwareVersionLabel(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                               size_t *num_responses, uint16_t *nack_reason);
  bool getEndpointList(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                       size_t *num_responses, uint16_t *nack_reason);
  bool getEndpointResponders(const uint8_t *param_data, uint8_t param_data_len, RdmParamData *resp_data_list,
                             size_t *num_responses, uint16_t *nack_reason);

  /* GET/SET RESPONSE PROCESSING */

  // RDMnet messages
  void endpointList(uint16_t conn, uint32_t changeNumber, const std::vector<std::pair<uint16_t, uint8_t>> &list,
                    const RdmUid &src_uid);
  void endpointResponders(uint16_t conn, uint16_t endpoint, uint32_t changeNumber, const std::vector<RdmUid> &list,
                          const RdmUid &src_uid);
  void endpointListChange(uint16_t conn, uint32_t changeNumber, const RdmUid &src_uid);
  void responderListChange(uint16_t conn, uint32_t changeNumber, uint16_t endpoint, const RdmUid &src_uid);

  // RDM PID GET responses/updates
  void nack(uint16_t conn, uint16_t reason, const RdmResponse *resp);
  void status(uint8_t type, uint16_t messageId, uint16_t data1, uint16_t data2, const RdmResponse *resp);

  // E1.20
  // SUPPORTED_PARAMETERS
  void commands(uint16_t conn, std::vector<uint16_t> list, const RdmResponse *resp);
  // DEVICE_INFO
  void deviceInfo(uint16_t conn, uint16_t protocolVersion, uint16_t modelId, uint16_t category, uint32_t swVersionId,
                  uint16_t footprint, uint8_t personality, uint8_t totalPersonality, uint16_t address,
                  uint16_t subdeviceCount, uint8_t sensorCount, const RdmResponse *resp);
  // DEVICE_MODEL_DESCRIPTION
  void modelDesc(uint16_t conn, const char *label, const RdmResponse *resp);
  // MANUFACTURER_LABEL
  void manufacturerLabel(uint16_t conn, const char *label, const RdmResponse *resp);
  // DEVICE_LABEL
  void deviceLabel(uint16_t conn, const char *label, const RdmResponse *resp);
  // SOFTWARE_VERSION_LABEL
  void softwareLabel(uint16_t conn, const char *label, const RdmResponse *resp);
  // BOOT_SOFTWARE_VERSION_ID
  void bootSoftwareID(uint16_t conn, uint32_t id, const RdmResponse *resp);
  // BOOT_SOFTWARE_VERSION_LABEL
  void bootSoftwareLabel(uint16_t conn, const char *label, const RdmResponse *resp);
  // DMX_START_ADDRESS
  void address(uint16_t conn, uint16_t address, const RdmResponse *resp);
  // IDENTIFY_DEVICE
  void identify(uint16_t conn, bool enable, const RdmResponse *resp);
  // DMX_PERSONALITY (number may equal zero if data is not provided)
  void personality(uint16_t conn, uint8_t current, uint8_t number, const RdmResponse *resp);
  // DMX_PERSONALITY_DESCRIPTION
  void personalityDescription(uint16_t conn, uint8_t personality, uint16_t footprint, const char *description,
                              const RdmResponse *resp);

  // E1.33
  // COMPONENT_SCOPE
  void componentScope(uint16_t conn, uint16_t scopeSlot, const char *scopeString, const char *staticConfigV4,
                      const char *staticConfigV6, uint16_t port, const RdmResponse *resp);
  // SEARCH_DOMAIN
  void searchDomain(uint16_t conn, const char *domainNameString, const RdmResponse *resp);
  // TCP_COMMS_STATUS
  void tcpCommsStatus(uint16_t conn, const char *scopeString, const char *v4AddrString, const char *v6AddrString,
                      uint16_t port, uint16_t unhealthyTCPEvents, const RdmResponse *resp);

  // Message sending
  void addPropertyEntries(RDMnetNetworkItem *parent, PIDFlags location);
  void initializeResponderProperties(ResponderItem *parent, uint16_t manuID, uint32_t deviceID);
  void initializeRPTClientProperties(RDMnetClientItem *parent, uint16_t manuID, uint32_t deviceID,
                                     rpt_client_type_t clientType);
  void sendGetControllerScopeProperties(uint16_t conn, uint16_t manuID, uint32_t deviceID);
  void sendGetNextControllerScope(uint16_t conn, uint16_t manuID, uint32_t deviceID, uint16_t currentSlot);
  void sendGetCommand(BrokerItem *brokerItem, uint16_t pid, uint16_t manu, uint32_t dev);
  uint8_t *packIPAddressItem(const QVariant &value, lwpa_iptype_t addrType, uint8_t *packPtr, bool packPort = true);

  // PID handling
  bool pidSupportedByGUI(uint16_t pid, bool checkSupportGet);

  // Item handling
  RDMnetClientItem *getClientItem(uint16_t conn, const RdmResponse *resp);
  RDMnetNetworkItem *getNetworkItem(uint16_t conn, const RdmResponse *resp);
  void checkPersonalityDescriptions(RDMnetNetworkItem *device, uint8_t numberOfPersonalities, const RdmResponse *resp);
  QVariant getPropertyData(RDMnetNetworkItem *parent, unsigned short pid, int role);

  class PropertyItem *createPropertyItem(RDMnetNetworkItem *parent, const QString &fullName);
  QString getShortPropertyName(const QString &fullPropertyName);
  QString getHighestGroupName(const QString &pathName);
  QString getPathSubset(const QString &fullPath, int first, int last = -1);
  class PropertyItem *getGroupingItem(RDMnetNetworkItem *parent, const QString &groupName);
  class PropertyItem *createGroupingItem(RDMnetNetworkItem *parent, const QString &groupName);
  QString getChildPathName(const QString &superPathName);
  QString getScopeSubPropertyFullName(RDMnetClientItem *client, uint16_t pid, int32_t index, const char *scope);

  void removeScopeSlotItemsInRange(RDMnetNetworkItem *parent, std::vector<class PropertyItem *> *properties,
                                   uint16_t firstSlot, uint16_t lastSlot);

public:
  RDMnetNetworkModel(RDMnetLibInterface *library, ControllerLog *log);
  ~RDMnetNetworkModel();
};
