#include <OpCoreCommon.hh>

using namespace std;

WaitingType OpCore::handleMessage(bool rdma, message_t *incoming_msg) {
    //cout << "error. fell through to base class" << endl;
}



// WaitingType OpCore::UpdateTarget(OpArgs *args) {
  

//     switch(state){

//         case State::start: {
//             //cout << "about to State::start and add timing point " << getOpTimingConstant() + SERVER + START << endl;
//             add_timing_point(getOpTimingConstant() + SERVER + START);

//             message_t *incoming_msg =  args->ExpectMessageOrDie<message_t *>(&peer);

//             dst = incoming_msg->src;
//             dst_mailbox = incoming_msg->src_mailbox;

//             return handleMessage(false, incoming_msg);
//         }
//         case State::done:
//             return WaitingType::done_and_destroy;
//     }

// }

WaitingType OpCore::UpdateTarget(OpArgs *args) {
  

    switch(state){

        case State::start: {
            //cout << "about to State::start and add timing point " << getOpTimingConstant() + SERVER + START << endl;
            add_timing_point(getOpTimingConstant() + SERVER + START);

            message_t *incoming_msg =  args->ExpectMessageOrDie<message_t *>(&peer);

            dst = incoming_msg->src;
            dst_mailbox = incoming_msg->src_mailbox;

            //cout << "msg: " << incoming_msg->body << endl;
            //cout << "msg length: " << incoming_msg->body_len << endl;
            if(incoming_msg->body[0] != '1') { //not rdma
                //cout << "about to handleMessage" << endl;
                return handleMessage(false, incoming_msg);
            }
            else {
                //cout << "about to handleRDMA" << endl;
                return handleRDMA(incoming_msg);
            }
        }
        case State::get_wait_complete:
            add_timing_point(getOpTimingConstant() + SERVER + RDMA_GET_FINISHED);
            //cout << "about to handleMessage after rdma" << endl;
            return handleMessage(true);

        case State::done:
            return WaitingType::done_and_destroy;
    }

}


WaitingType OpCore::UpdateOrigin(OpArgs *) {
    return WaitingType::error;

}

WaitingType OpCore::handleRDMA(message_t *incoming_msg)
{
    //cout << "incoming_msg->body[1]: " << incoming_msg->body[1] << endl;

    // save a copy of the NBR for later use, don't grab the rdma flag bit
    memcpy(&nbr, &incoming_msg->body[1], sizeof(opbox::net::NetBufferRemote));

    //cout << "nbr addr: " << &nbr << endl;
    // this is the initiator buffer for the get and the put
    shout_ldo = lunasa::DataObject(0, nbr.GetLength(), lunasa::DataObject::AllocatorType::eager);

    //cout << "about to opbox::net::Get" << endl;
    // get the ping message from the origin process.
    // AllEventsCallback() is a convenience class that will redirect
    // all events generated by the get to this operation's Update()
    // method.
    opbox::net::Get(peer, &nbr, shout_ldo, AllEventsCallback(this));



    add_timing_point(getOpTimingConstant() + SERVER + RDMA_GET_START);
    //cout << "handleRDMA finished" << endl;
    state=State::get_wait_complete;
    return WaitingType::waiting_on_cq;

}

std::string OpCore::serializeMsgToClient(uint64_t id,
                                                  int return_value) {

    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << id;
    oa << return_value;
    //log("the archived message is " + ss.str());


    //cout << "Server just serialized message to client \n";
    return ss.str();
}