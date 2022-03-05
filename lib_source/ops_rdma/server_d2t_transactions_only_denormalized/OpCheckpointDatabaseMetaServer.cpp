#include <OpCheckpointDatabaseMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;
// extern int proc_rank;
// extern int checkpt_count;

// int sql_output_db(sqlite3 *pInMemory, uint64_t job_id, int rank, int checkpt_count);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

int db_checkpoint_copy(uint64_t job_id);
int db_checkpoint_incremental_output(uint64_t job_id);
int db_checkpoint_copy_and_delete(uint64_t job_id);
int db_checkpoint_copy_and_reset(uint64_t job_id);
int db_checkpoint_incremental_output_and_delete(uint64_t job_id);
int db_checkpoint_incremental_output_and_reset(uint64_t job_id);

using namespace std;

WaitingType OpCheckpointDatabaseMeta::handleMessage(bool rdma, message_t *incoming_msg) {
	uint64_t job_id;
	int checkpt_type;
	int rc, rc2;
    char  *ErrMsg = NULL;

    md_checkpoint_database_args sql_args;

	// cout << "message: " << incoming_msg->body << endl;
 //    cout << "message: " << incoming_msg->body << endl;
	// cout << "about to scanf" << endl;
	// sscanf(incoming_msg->body, "%llu/%d", &job_id, &checkpt_type);
	// cout << "job_id: " << job_id << " checkpt_type: " << checkpt_type << endl;
	// uint64_t job_id = stoull(incoming_msg->body,nullptr,0);

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
	add_timing_point(OP_CHECKPOINT_DATABASE_DEARCHIVE_MSG_FROM_CLIENT);

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error OpCheckpointDatabaseMeta Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

	switch(sql_args.checkpt_type) {
		case DB_COPY: {
			rc = db_checkpoint_copy(sql_args.job_id);
			break;
		}
		case DB_INCR_OUTPUT: {
			rc = db_checkpoint_incremental_output(sql_args.job_id);
			break;
		}	   		
		case DB_COPY_AND_DELETE: {
			rc = db_checkpoint_copy_and_delete(sql_args.job_id);
			break;
		}	    		
		case DB_COPY_AND_RESET: {
			rc = db_checkpoint_copy_and_reset(sql_args.job_id);
			break;
		}	    		
		case DB_INCR_OUTPUT_AND_DELETE: {
			rc = db_checkpoint_incremental_output_and_delete(sql_args.job_id);
			break;
		}	    		
		case DB_INCR_OUTPUT_AND_RESET: {
			rc = db_checkpoint_incremental_output_and_reset(sql_args.job_id);
			break;
		}
	}
    //keep the return value separate so we pass the correct checkpoint return value to the client
    //makes sure we know when db reset has occurred
    rc2 = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc2 != SQLITE_OK)
    {
        fprintf (stderr, "Error OpCheckpointDatabaseMeta Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        rc = rc2; //make sure we return an error code to the client
    }

	add_timing_point(OP_CHECKPOINT_DATABASE_MD_CHECKPOINT_DATABASE_STUB);

cleanup:
    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;

    // checkpt_count += 1;
    add_timing_point(OP_CHECKPOINT_DATABASE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;
}

