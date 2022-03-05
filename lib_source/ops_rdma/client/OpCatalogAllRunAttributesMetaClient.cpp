

#include <OpCatalogAllRunAttributesMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;


OpCatalogAllRunAttributesMeta::OpCatalogAllRunAttributesMeta(opbox::net::peer_ptr_t dst, const md_catalog_all_run_attributes_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_START);

    // cout << "Client about to serialize a message to server \n";

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_SERIALIZE_MSG_FOR_SERVER);
    // cout << "Client about to create outgoing message to server \n";

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);

    // add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
  // cout << "Client just dispatched message to server \n";

}

WaitingType OpCatalogAllRunAttributesMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}
