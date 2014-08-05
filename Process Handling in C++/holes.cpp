#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <queue>
#include <map>
#include <numeric>
#include <string.h>

using namespace std;
// Fit Strategy
enum FIT_TYPE
{
  FIRST_FIT,
  BEST_FIT,
  NEXT_FIT,
  WORST_FIT
};
// ProcessInfo
struct ProcessInfo
{
  std::pair<int, int> range;// memroy start pos and end pos
  int history;// history in memory
  int swapOutTimes;// how many times this process has been swapped out
  bool isInMemory;// whether this process is in memory
};
// Stats for recoding statistical data
struct Stats
{
  int memUsageSum;
  int holesSum;
  int processSum;
  int loadTimes;
  
  int memUsed;
  int holes;
};
// read process info from data file
bool readProcesses(std::string&, std::queue<std::pair<std::string, int> >&);
// simute a fit stratergy for memeory management
void simulate(const FIT_TYPE&, std::queue<std::pair<std::string, int> >&, bitset<128>&);
// Allocate memory block for process
void allocateMemory(bitset<128>&, std::map<std::string, ProcessInfo>&, const std::string&, const int&, const int&);
// Search empty memory block
int search(const FIT_TYPE&, bitset<128>&, const int&, int&);
// Find the process in memory which has longest history
void findLongestPid(std::map<std::string, ProcessInfo>&, std::string&);
// Print processInfoMap
void printMap(std::map<std::string, ProcessInfo>&);
// Calculate statistical info
void calculateStats(const std::map<std::string, ProcessInfo>&, const bitset<128>&, int&, int&, int&);
// Collection statistical info
void collectStats(std::map<std::string, ProcessInfo>&, bitset<128>&, Stats&);

int main(int argc, char** argv)
{
  // Check argument parameters
  if(argc != 2)
  {
    cout << "Please enter the input file\n";
    return 1;
  }
  // memory status used or not
  bitset<128> memStatus(0);
  // process queue waiting to be swapped in
  std::queue<std::pair<std::string, int> > psQueue;
  // File name
  std::string fileName = argv[1];
  if(!readProcesses(fileName, psQueue))
  {
    cout << "Load process file error\n";
    return 1;
  }
  // vec for different fit stratergy for memory management
  std::vector<FIT_TYPE> typeVec;
  typeVec.push_back(FIRST_FIT);
  typeVec.push_back(BEST_FIT);
  typeVec.push_back(NEXT_FIT);
  typeVec.push_back(WORST_FIT);
  // name map for fit stratergy
  std::map<FIT_TYPE, std::string> typeNameMap;
  typeNameMap[FIRST_FIT] = "First Fit";
  typeNameMap[BEST_FIT] = "Best Fit";
  typeNameMap[NEXT_FIT] = "Next Fit";
  typeNameMap[WORST_FIT] = "Worst Fit";
  // Perform different fit stratergy simulation
  for(std::vector<FIT_TYPE>::iterator it = typeVec.begin(); it != typeVec.end(); it++)
  {
    FIT_TYPE& fType = (*it);
    cout << typeNameMap[fType] << " Simulation" << endl;
    simulate(FIRST_FIT, psQueue, memStatus);
    cout << endl;
  }
  
  return 0;
}
// read process info from data file
bool readProcesses(std::string& fileName, std::queue<std::pair<std::string, int> >& psQueue)
{
  fstream fin(fileName.c_str());
  if (!fin)
  {
    cout << "open file " << fileName << " error\n";
    return false;
  }
 
  std::string pid = "";
  int size = 0;
   
  while (!fin.eof())
  {
    fin >> pid >> size;
    psQueue.push(std::make_pair(pid, size));  
  }

  fin.close();
  return true;
}
// silmulate a specific fit stratergy
void simulate(const FIT_TYPE& fType, std::queue<std::pair<std::string, int> >& psQueue, bitset<128>& memStatus)
{
  // Copy a temp queue
  std::queue<std::pair<std::string, int> > tQueue = psQueue;
  // Process info map
  std::map<std::string, ProcessInfo> psInfoMap;
  // Statistics info
  Stats s;
  // Clear statistics info
  memset(&s, 0, sizeof(Stats));
  int totalMem = memStatus.size();
  int start = 0;
  // Clear memory avaialble status
  for(size_t i = 0; i < memStatus.size(); i++)
  {
    memStatus.set(i, 0);
  }
  
  while(!tQueue.empty())
  {
    std::pair<std::string, int>& tPair = tQueue.front();
    tQueue.pop();
    // Process id
    std::string& pid = tPair.first;
    // Process size
    int& size = tPair.second;
    // Search for the position for a valid memory block
    int pos = search(fType, memStatus, size, start);
    if(pos >= 0)
    {
      // allocate memory
      allocateMemory(memStatus, psInfoMap, pid, size, pos);
      //printMap(psInfoMap);
      collectStats(psInfoMap, memStatus, s);
      double memUsage = s.memUsed / (0.0+totalMem);
      double avgMemUsage = s.memUsageSum/(0.0+s.loadTimes)/(0.0+totalMem);
      cout << "pid loaded, #processes = " << pid << ", #holes = " 
      << s.holes << ", %memusage = " << int(memUsage * 100) << ", cumulative %mem = " << int(avgMemUsage * 100) << endl;
    }
    else
    {
      int pos = 0;
      do
      {
        std::string lpid = "";
        // Find the process id of process in memory with longest history
        findLongestPid(psInfoMap, lpid);
        std::pair<int, int>& range = psInfoMap[lpid].range;
        // Clear memory status
        int lSize = range.second-range.first + 1;
        for(int i = range.first; i <= range.second; i++)
        {
          memStatus.set(i,0);
        }
        psInfoMap[lpid].isInMemory = false;
        psInfoMap[lpid].history = 0;
        psInfoMap[lpid].swapOutTimes++;
        // if a process has been swapped out three times
        if(psInfoMap[lpid].swapOutTimes < 3)
        {
          tQueue.push(std::make_pair(lpid, lSize));
        }
        //printMap(psInfoMap);
        // Search available memory block
        pos = search(fType, memStatus, size, start);
        if(pos >= 0)
        {
          break;
        }
      }
      while(1);
      
      if(pos >= 0)
      {
        allocateMemory(memStatus, psInfoMap, pid, size, pos);
        //printMap(psInfoMap);
        collectStats(psInfoMap, memStatus, s);
        double memUsage = s.memUsed / (0.0+totalMem);
        double avgMemUsage = s.memUsageSum/(0.0+s.loadTimes)/(0.0+totalMem);
        cout << "pid loaded, #processes = " << pid << ", #holes = " 
        << s.holes << ", %memusage = " << int(memUsage * 100) << ", cumulative %mem = " << int(avgMemUsage * 100) << endl;
      }
    }
  }
  
  double avgMemUsage = s.memUsageSum/(0.0+s.loadTimes)/(0.0+totalMem);
  cout << "Total loads = " << s.loadTimes << ", average #processes = " << s.processSum/(0.0+s.loadTimes) << ", average #holes = " << s.holesSum/(0.0+s.loadTimes) << ", cumulative %mem = " << int(avgMemUsage * 100) << endl;
}
// allocate memeory block for a specific process
void allocateMemory(bitset<128>& memStatus, std::map<std::string, ProcessInfo>& psInfoMap, const std::string& pid, const int& size, const int& pos)
{
  for(int i = 0; i < size; i++)
  {
    // Set memory status
    memStatus.set(pos+i,1);
  }
   
  for(std::map<std::string, ProcessInfo>::iterator it = psInfoMap.begin(); it != psInfoMap.end(); it++)
  {
    ProcessInfo& psInfo = it->second;
    if(psInfo.isInMemory)
    {
      // Increase history of processes which already exist in memory
      psInfo.history++; 
    }
  }
  
  std::map<std::string, ProcessInfo>::iterator tit = psInfoMap.find(pid);
  if(tit != psInfoMap.end())
  {
    // The process has been swapped out and process info exist in psInfoMap
    tit->second.isInMemory = true;
    tit->second.history = 0;
    tit->second.range.first = pos;
    tit->second.range.second = pos+size-1;
  }
  else
  {
    // The process has not been swapped in
    ProcessInfo psInfo;
    psInfo.isInMemory = true;
    psInfo.history = 0;
    psInfo.range.first = pos;
    psInfo.range.second = pos+size-1;
    psInfo.swapOutTimes = 0;
    psInfoMap[pid] = psInfo;
  }
}
// find the process id in memory with the longest history
void findLongestPid(std::map<std::string, ProcessInfo>& psInfoMap, std::string& pid)
{
  int maxHistory = -1;
  
  for(std::map<std::string, ProcessInfo>::iterator it = psInfoMap.begin(); it != psInfoMap.end(); it++)
  {
    ProcessInfo& psInfo = it->second;
    if(psInfo.isInMemory && psInfo.history > maxHistory)
    {
      maxHistory = psInfo.history;
      pid = it->first;
    }
  }
}
// Search the available memory block based on different stratergy
int search(const FIT_TYPE& fType, bitset<128>& memStatus, const int& size, int& start)
{
  size_t mSize = memStatus.size();
  
  if(fType == FIRST_FIT)
  {
    // First fit and search the first valid memory block
    int pos = 0;
    int count = 0;
    
    for(size_t i = 0; i < mSize; i++)
    {
      if(!memStatus.test(i))
      {
        if(count == 0)
        {
          pos = i;
        }
      
        if(count >= size)
        {
          return pos;
        }
        else
        {
          count++;
        }
      }
      else
      {
        count = 0;
      }
    }
    return count >= size ? pos : -1;
  }
  else if(fType == BEST_FIT)
  {
    // Best fit and search the first valid memory block with the least space waste
    int pos = 0;
    int count = 0;
    int best_pos = -1;
    int min_diff = mSize;
    
    for(size_t i = 0; i < mSize; i++)
    {
      if(!memStatus.test(i))
      {
        if(count == 0)
        {
          pos = i;
        }
      
        if(count >= size)
        {
          if(count-size < min_diff)
          {
            min_diff = count-size;
            best_pos = pos;
          }
        }
        else
        {
          count++;
        }
      }
      else
      {
        count = 0;
      }
    }
    return best_pos;
  }
  else if(fType == NEXT_FIT)
  {
    // Similar like first fit and start search the first valid memory block from the last pos which has been probed
    int pos = 0;
    int count = 0;
    size_t start_pos = start;
    bool found = false;
    for(size_t i = start_pos; i < mSize; i++)
    {
      if(!memStatus.test(i))
      {
        if(count == 0)
        {
          pos = i;
        }
      
        if(count >= size)
        {
          start = pos;
          found = true;
          break;
        }
        else
        {
          count++;
        }
      }
      else
      {
        count = 0;
      }
    }
    
    if(found)
    {
      return count >= size ? pos : -1;
    }
    else
    {
      pos = 0;
      count = 0;
      for(size_t i = 0; i < mSize; i++)
      {
        if(!memStatus.test(i))
        {
          if(count == 0)
          {
            pos = i;
          }
      
          if(count >= size)
          {
            start = pos;
            found = true;
            break;
          }
          else
          {
            count++;
          }
        }
        else
        {
          count = 0;
        }
      }
      
      if(found)
      {
        return count >= size ? pos : -1;
      }
      else
      {
        return -1;
      }
    }
  }
  else if(fType == WORST_FIT)
  {
    // Worst fit and search the valid memory block with largest size
    int pos = 0;
    int count = 0;
    int worst_pos = -1;
    int max_diff = -1;
    
    for(size_t i = 0; i < mSize; i++)
    {
      if(!memStatus.test(i))
      {
        if(count == 0)
        {
          pos = i;
        }
      
        if(count >= size)
        {
          if(count-size > max_diff)
          {
            max_diff = count-size;
            worst_pos = pos;
          }
        }
        else
        {
          count++;
        }
      }
      else
      {
        count = 0;
      }
    }
    return worst_pos;
  }
  
  return -1;
}
// Print process info map
void printMap(std::map<std::string, ProcessInfo>& psInfoMap)
{
  for(std::map<std::string, ProcessInfo>::iterator it = psInfoMap.begin(); it != psInfoMap.end(); it++)
  {
    const std::string& pid = it->first;
    ProcessInfo& psInfo = it->second;
    cout << "pid:" << pid << ", start:" << psInfo.range.first << ", end:" << psInfo.range.second << ", history:" << psInfo.history << ", swap:" << psInfo.swapOutTimes << ", inMemeory:" << psInfo.isInMemory << endl;
  }
}
// Calculate statistics info
void calculateStats(std::map<std::string, ProcessInfo>& psInfoMap, bitset<128>& memStatus, int& memUsed, int& processNum, int& holes)
{
  int sum = 0;
  int count = 0;
  int holesNum = 0;
  
  for(std::map<std::string, ProcessInfo>::iterator it = psInfoMap.begin(); it != psInfoMap.end(); it++)
  {
    const ProcessInfo& psInfo = it->second;
    if(psInfo.isInMemory)
    {
      // sum up memory allocation
      sum += (psInfo.range.second-psInfo.range.first+1);
      count++;
    }
  }
  
  bool hasOne = false;
  int start = 0;
  int len = 0;
  size_t mSize = memStatus.size();
  // Calculate holes number which contains consecutive zero(s)
  for(size_t pos = 0; pos < mSize; pos++)
  {
    if(!memStatus.test(pos))
    {
      if(len == 0)
      {
        start = pos;
      }
      len++;
    }
    else
    {
      if(start >= 0 and len > 0)
      {
        holesNum++;
        start = 0;
        len = 0;
      }
      hasOne = true;
    }
  }
  
  if(!hasOne)
  {
    holesNum = 0;
  }
  else if(start >= 0 and len > 0)
  {
    holesNum++;
  }
 
  memUsed = sum;
  processNum = count;
  holes = holesNum;
}
// Collect statistics info
void collectStats(std::map<std::string, ProcessInfo>& psInfoMap, bitset<128>& memStatus, Stats& s)
{
  int memUsed = 0;
  int processNum = 0;
  int holes = 0;
  // Calculate statistics info
  calculateStats(psInfoMap, memStatus, memUsed, processNum, holes);
 
  s.loadTimes++;
  s.memUsageSum += s.memUsed;
  s.holesSum += holes;
  s.processSum += processNum;
  
  s.memUsed = memUsed;
  s.holes = holes;
}
