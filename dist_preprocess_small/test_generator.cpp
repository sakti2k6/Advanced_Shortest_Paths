#include <iostream>
#include <vector>

using namespace std;

int main() 
{ 
  
    // Use current time as seed for random generator 
    srand(time(0)); 
    int n = 1070376; 
    for(int i = 0; i < 10000; i++) 
        cout << rand() % 1070376 << " " << rand() % 1070376 << endl; 
  
    return 0; 
}
