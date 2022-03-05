


#ifndef EVALTESTINGOUTPUT_HH
#define EVALTESTINGOUTPUT_HH

// #include <stdint.h>
#include <map>

struct testing_config {
	uint16_t num_write_client_procs;
	uint16_t num_write_client_nodes;
	uint16_t num_read_client_procs;
	uint16_t num_read_client_nodes;
	uint64_t num_datasets;
	// uint16_t num_types;

	// uint16_t num_storage_pts; // just used by server
}; 

struct pt_of_interest {
	uint16_t start_code;
	uint16_t end_code;
	std::string name;

	pt_of_interest ( std::string point_name, uint16_t start, uint16_t end) {
		start_code = start;
		end_code = end;
		name = point_name;
	}
};

// struct timing_pt {
// 	std::string point_name;
// 	long double time;
// 	long double total_op_time;

// 	timing_pt ( std::string name, long double time_pt, long double op_time) {
// 		point_name = name;
// 		time = time_pt;
// 		total_op_time = op_time;
// 	}
// };

struct timing_pts {
	std::string name;
	std::vector<long double> time_pts;
	// vector<long double> total_op_time_pts;

	timing_pts ( std::string pt_name, std::vector<long double> pts) {
		name = pt_name;
		time_pts = pts;
	}
};


struct client_config_output {
	testing_config config; 

	std::vector<std::string> filenames;

	std::vector<timing_pts> per_proc_op_times;
	std::vector<timing_pts> per_proc_times;
	std::vector<timing_pts> last_first_times;

	std::vector<std::vector<long double>> op_times;

	// std::vector<std::vector<std::vector<long double>>> op_times;
};

// struct read_client_config_output {
// 	testing_config config; 

// 	std::vector<std::string> filenames;

// 	std::vector<timing_pts> per_proc_op_times;
// 	std::vector<timing_pts> per_proc_times;
// 	std::vector<timing_pts> last_first_times;

// 	std::vector<std::vector<std::vector<long double>>> op_times;
// };


   
//for debug testing purposes
struct debugLog {
  private:
    bool on;

  public:
  debugLog(bool turn_on) {
    on = turn_on;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> debugLog& operator << (const T& x) {
   if(on) {
      std::cout << x;
    }
    return *this;
  }
  debugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on) {
      std::cout << manipulator;
    }
     return *this;
  }
};


#endif //EVALTESTINGOUTPUT_HH