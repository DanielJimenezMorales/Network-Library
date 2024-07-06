#include "PeerConnectivityTests.h"
#include "ReplicationTests.h"
#include "LogTestUtils.h"

int main()
{
    Tests::PeerConnectivityTests::ExecuteAll();
    //Tests::ReplicationTests::ExecuteAll();
    return EXIT_SUCCESS;
}
