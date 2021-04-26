#include "/path/to/llvm/projects/compiler-rt/lib/sanitizer_common/sanitizer_common.h"
#include "/path/to/llvm/projects/compiler-rt/lib/sanitizer_common/sanitizer_flags.h"
#include "/path/to/llvm/projects/compiler-rt/lib/sanitizer_common/sanitizer_libc.h"
#include "/path/to/llvm/projects/compiler-rt/lib/sanitizer_common/sanitizer_report_decorator.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unordered_map>
#include <set>
#include <vector>
#include <numeric>
#include <chrono>
#include <iomanip>
#include<ctime>
using namespace std;

//********************************************************************//
//******** For Direct Call log ***************************************//


/*unordered_map<char*, set<char*>> *target_set = nullptr;
std::vector<string>*time_log = nullptr;
char *path = "/path/to/file.txt";

bool outputTargetSet = false;

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void target(char* call, char* target) {

  //Initialize the data structure if it doesn't exist
  if(target_set == nullptr){
    assert(!outputTargetSet && "[HexCFI] trying to make new target set after logging the first one\n");
    target_set = new unordered_map<char *, set<char *>>();
  }

  //Do we need a new entry in the map?
 	if(target_set->find(call) == target_set->end()) {
		//printf("[HexCFI] new callsite\n");
		set<char*> tset;
    tset.insert(target);
		target_set->insert(std::pair<char*, set<char*>>(call, tset));
	}
  else {
		//printf("[HexCFI] new target for callsite\n");
		target_set->find(call)->second.insert(target);
	}
}



extern "C" SANITIZER_INTERFACE_ATTRIBUTE
__attribute__((destructor(0)))
void destroy_hexcfi() {
  std::chrono::high_resolution_clock m_clock;
  uint64_t time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(m_clock.now().time_since_epoch()).count();

  FILE *status = fopen("/proc/self/status", "r");
  assert(status && "[HexCFI] failed to open /proc/self/status for reading\n");
  char buf[256];
  fgets(buf, 256, status);
  int len = strlen(buf);
  buf[len - 1] = '\0';  //remove the newline
  
  //Buf now holds the executable name, make our target set log
  char *log_name = (char *)calloc(len+strlen(path)+50, 1);
  strcat(log_name, path);
  strcat(log_name, buf+6); //First line of status is: ^Name:\t<prog name>\n$"
  strcat(log_name,std::to_string(time_nano).c_str());
  FILE *print_log = fopen(log_name, "w");
  free(log_name);

  assert(print_log && "[HexCFI] Failed to open the target set log file\n");
  

  if (target_set) {
  for(auto it = target_set->begin(); it != target_set->end(); ++it) {
    fprintf(print_log, "%s", it->first);
		for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
      fprintf(print_log, ",%s", *it2);
		}
    fprintf(print_log, "\n");
	}
  }
  fclose(print_log);


  delete target_set;
  target_set = nullptr;
  outputTargetSet = true;
}
*/
//********************************************************************//
//******** End of Direct Call log ***************************************//

//********************************************************************//
//******** Begin of Indirect Call log ********************************//



unordered_map<char*, set<void*>> *target_set = nullptr;
unordered_map<void*, vector<char*>> *function_map = nullptr;
std::vector<string>*time_log = nullptr;
char *path = "/path/to/file.txt";
char *pathtime = "/path/to/file.txt";
bool outputTargetSet = false;

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void hexcfi_function_map(char *name, void *addr){
  if(!function_map){
    assert(!outputTargetSet && "[HexCFI] trying to map a function after outputing the mappings\n");
    function_map = new unordered_map<void *, vector<char *>>();
  }
 	if(function_map->find(addr) == function_map->end()) {
		vector<char *> fnames;
                fnames.push_back(name);
		function_map->insert(std::pair<void*, vector<char*>>(addr, fnames));
		
	}
  else {
		function_map->find(addr)->second.push_back(name);
		
	}
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void target(char* call, void* target) {

  //Initialize the data structure if it doesn't exist
  if(target_set == nullptr){
    assert(!outputTargetSet && "[HexCFI] trying to make new target set after logging the first one\n");
    target_set = new unordered_map<char *, set<void *>>();
  }

  //Do we need a new entry in the map?
 	if(target_set->find(call) == target_set->end()) {
		set<void*> tset;
    tset.insert(target);
		target_set->insert(std::pair<char*, set<void*>>(call, tset));
	}
  else {
		target_set->find(call)->second.insert(target);
	}
  /* for timestamp */
  std::chrono::high_resolution_clock m_clock; 
  uint64_t time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(m_clock.now().time_since_epoch()).count();
  char temp_log[1024]; 
  sprintf(temp_log,"%lu:%s:%p",time_nano,call,target);
  if(time_log == nullptr) {
    time_log = new vector<string>();
  }
  
  std::string Tlog = std::string(temp_log);
  time_log->push_back(Tlog);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
__attribute__((destructor(0)))
void destroy_hexcfi() {
  std::chrono::high_resolution_clock m_clock;
  uint64_t time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(m_clock.now().time_since_epoch()).count();

  FILE *status = fopen("/proc/self/status", "r");
  assert(status && "[HexCFI] failed to open /proc/self/status for reading\n");
  char buf[256];
  fgets(buf, 256, status);
  int len = strlen(buf);
  buf[len - 1] = '\0';  //remove the newline
  
  //Buf now holds the executable name, make our target set log
  char *log_name = (char *)calloc(len+strlen(path)+50, 1);
  strcat(log_name, path);
  strcat(log_name, buf+6); //First line of status is: ^Name:\t<prog name>\n$"
  strcat(log_name,std::to_string(time_nano).c_str());
  FILE *print_log = fopen(log_name, "w");
  free(log_name);

  char *logtime = (char *)calloc(len+strlen(pathtime)+50, 1);
  strcat(logtime, pathtime);
  strcat(logtime, buf+6);
  strcat(logtime,std::to_string(time_nano).c_str());
  FILE *print_timeLog = fopen(logtime, "w");
  free(logtime);
  
  //Buf now holds the executable name, make our function map log
  char *map_name = (char *)calloc(len+strlen(path)+50, 1);
  strcat(map_name, path);
  strcat(map_name, buf+6);
  strcat(map_name,std::to_string(time_nano).c_str());
  strcat(map_name, "-map");
  FILE *fname_map = fopen(map_name, "w");
  free(map_name);

  assert(print_log && "[HexCFI] Failed to open the target set log file\n");
  assert(fname_map && "[HexCFI] Failed to open the function map log\n");
  if(time_log) {
  for(auto it = time_log->begin(); it != time_log->end(); ++it) {
  		fprintf(print_timeLog,"%s\n",it->c_str()); 	
  	}

  }

  if (target_set) {
  for(auto it = target_set->begin(); it != target_set->end(); ++it) {
    fprintf(print_log, "%s", it->first);
		for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
      fprintf(print_log, ", %p", *it2);
		}
    fprintf(print_log, "\n");
	}
  }
  fclose(print_log);
   if(function_map) {
  for(auto it3 = function_map->begin(); it3 != function_map->end(); ++it3){
    fprintf(fname_map, "%p", it3->first);
    for(auto it4 = it3->second.begin(); it4 != it3->second.end(); ++it4)
      fprintf(fname_map, ", %s", *it4);
    fprintf(fname_map, "\n");
  } 
 }
  fclose(fname_map);

  delete target_set;
  delete function_map;
  target_set = nullptr;
  function_map = nullptr;
  outputTargetSet = true;
}
