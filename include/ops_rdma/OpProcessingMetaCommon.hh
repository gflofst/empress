


#ifndef OPPROCESSINGMETACOMMON_HH
#define OPPROCESSINGMETACOMMON_HH

#include <OpCoreCommon.hh>

class OpProcessingMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpProcessingMeta");
        inline const static std::string  op_name = "OpProcessingMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpProcessingMeta(opbox::net::peer_ptr_t peer, const md_processing_args &args);
  // OpProcessingMeta(op_create_as_target_t t);
  // ~OpProcessingMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();

private:
    unsigned short getOpTimingConstant() const { return MD_PROCESSING_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<int> return_msg_promise;
  //std::string serializeMsgToServer(const md_processing_args &args);
  //void deArchiveMsgFromClient(const std::string &serial_str, md_processing_args &args);



};

#endif // OPPROCESSINGMETACOMMON_HH
