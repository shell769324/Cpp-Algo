#include <vector>
using namespace std;
int main() {
    int* data = static_cast<int*>(::operator new(0));
    ::delete data;
}
