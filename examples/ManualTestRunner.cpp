// Educational example of a minimal custom C++ test runner.
// The production test suite uses GoogleTest.

#include <iostream>
#include <string>

#include "User.h"
#include "Call.h"
#include "Network.h"

void check(
    bool condition,
    const std::string &testName,
    int &passed,
    int &failed)
{
    if (condition)
    {
        std::cout << "[PASS] " << testName << "\n";
        ++passed;
    }
    else
    {
        std::cout << "[FAIL] " << testName << "\n";
        ++failed;
    }
}

void testUser(int &passed, int &failed)
{
    User user(1, "John Doe", "123-456-7890");
    check(user.getId() == 1, "User stores correct ID", passed, failed);
    check(user.getName() == "John Doe", "User stores correct name", passed, failed);
    check(user.getPhoneNumber() == "123-456-7890", "User stores correct phone number", passed, failed);
}

void testCall(int &passed, int &failed)
{
    Call createdCall(1, 1, 2);
    check(createdCall.getId() == 1, "Call has correct ID", passed, failed);
    check(createdCall.getCallerId() == 1, "Call has correct caller ID", passed, failed);
    check(createdCall.getReceiverId() == 2, "Call has correct receiver ID", passed, failed);
    check(createdCall.getStatusId() == CallStatus::Created, "Call has correct status", passed, failed);

    Call callEndedBeforeStart(2, 3, 4);
    check(callEndedBeforeStart.end() == false, "Call doesent end before it started", passed, failed);
    check(callEndedBeforeStart.getStatusId() == CallStatus::Created, "Call didnt change status from Created", passed, failed);

    Call activeCall(3, 5, 6);
    check(activeCall.start() == true, "Call starts properly", passed, failed);
    check(activeCall.getStatusId() == CallStatus::Active, "Call status properly changed to Active", passed, failed);
    check(activeCall.start() == false, "Call already started", passed, failed);
    check(activeCall.getStatusId() == CallStatus::Active, "Call didnt change status from Active", passed, failed);

    Call endedCall(4, 7, 8);
    check(endedCall.start() == true, "Call starts properly", passed, failed);
    check(endedCall.end() == true, "Call ended properly", passed, failed);
    check(endedCall.getStatusId() == CallStatus::Ended, "Call status properly changed to Ended", passed, failed);
    check(endedCall.end() == false, "Call already ended", passed, failed);
    check(endedCall.start() == false, "Call cant start after ending", passed, failed);
    check(endedCall.getStatusId() == CallStatus::Ended, "Call didnt changed status from Ended", passed, failed);
}

void testNetworkUserManagement(int &passed, int &failed)
{
    Network newNetwork;
    User firstUser(1, "John Doe", "123-456-7890");
    User secondUser(2, "Johnny Doesony", "123-456-7777");
    check(newNetwork.getUserCount() == 0, "Network is empty at start", passed, failed);
    check(newNetwork.addUser(firstUser) == true, "First user is added to the network", passed, failed);
    check(newNetwork.getUserCount() == 1, "User count correct after adding first user", passed, failed);
    check(newNetwork.addUser(firstUser) == false, "Trying to add the same user to Network", passed, failed);
    check(newNetwork.getUserCount() == 1, "User count correct after trying to duplicate first user", passed, failed);
    check(newNetwork.addUser(secondUser) == true, "Second user is added to the network", passed, failed);
    check(newNetwork.getUserCount() == 2, "User count correct after adding second user", passed, failed);
    check(newNetwork.removeUser(secondUser.getId()) == true, "Second user is removed from the network", passed, failed);
    check(newNetwork.getUserCount() == 1, "User count correct after removing second user", passed, failed);
    check(newNetwork.removeUser(secondUser.getId()) == false, "Second user cant be removed again from the network", passed, failed);
    check(newNetwork.getUserCount() == 1, "User count correct after trying to remove second user again", passed, failed);
    check(newNetwork.removeUser(999) == false, "Second user cant be removed again from the network", passed, failed);
    check(newNetwork.getUserCount() == 1, "User count correct after trying to remove non existing user", passed, failed);
}

void testNetworkCallCreation(int &passed, int &failed)
{
    Network callCreationTestNetwork;
    User firstTestUser(1, "John Doe", "123-456-7890");
    User secondTestUser(2, "Johnny Doesony", "123-456-7777");
    User thirdTestUser(3, "Johnini Doesini", "123-456-9999");
    callCreationTestNetwork.addUser(firstTestUser);
    callCreationTestNetwork.addUser(secondTestUser);
    callCreationTestNetwork.addUser(thirdTestUser);
    check(callCreationTestNetwork.getCallCount() == 0, "Call count is 0", passed, failed);
    check(callCreationTestNetwork.createCall(5, 1, 2) == true, "Creating first call between user 1 and user 2", passed, failed);
    check(callCreationTestNetwork.getCallCount() == 1, "Call count is 1 after first call", passed, failed);
    check(callCreationTestNetwork.createCall(6, 4, 2) == false, "Creating second call between not existing user and user 2 rejected", passed, failed);
    check(callCreationTestNetwork.getCallCount() == 1, "Call count is 1 after second call", passed, failed);
    check(callCreationTestNetwork.createCall(7, 2, 4) == false, "Creating third call between user 2 and not existing user rejected", passed, failed);
    check(callCreationTestNetwork.createCall(8, 1, 1) == false, "Creating fourth call between user 1 and himself rejected", passed, failed);
    check(callCreationTestNetwork.createCall(5, 2, 3) == false, "Creating call with already used ID", passed, failed);
    check(callCreationTestNetwork.createCall(9, 2, 3) == true, "Creating call between user 2 and user 3", passed, failed);
    check(callCreationTestNetwork.getCallCount() == 2, "Call count final is 2", passed, failed);
}

void testNetworkCallControl(int &passed, int &failed)
{
    Network callControlNetwork;
    User firstTestUser(1, "John Doe", "123-456-7890");
    User secondTestUser(2, "Johnny Doesony", "123-456-7777");
    User thirdTestUser(3, "Johnini Doesini", "123-456-9999");
    User fourthTestUser(4, "Johniasty Doesiniasty", "999-999-9999");
    callControlNetwork.addUser(firstTestUser);
    callControlNetwork.addUser(secondTestUser);
    callControlNetwork.addUser(thirdTestUser);
    callControlNetwork.addUser(fourthTestUser);
    callControlNetwork.createCall(10, 1, 2);
    callControlNetwork.createCall(11, 2, 3);
    callControlNetwork.createCall(12, 3, 4);
    check(callControlNetwork.getCallCount() == 3, "Network contains 3 calls before call control tests", passed, failed);
    check(callControlNetwork.getUserCount() == 4, "Network contains 4 users before call control tests", passed, failed);
    check(callControlNetwork.startCall(999) == false, "Starting call with invalid ID", passed, failed);
    check(callControlNetwork.startCall(999) == false, "Starting call with invalid ID", passed, failed);
    check(callControlNetwork.endCall(999) == false, "Ending call with invalid ID", passed, failed);
    check(callControlNetwork.startCall(10) == true, "Starting call with ID = 10", passed, failed);
    check(callControlNetwork.startCall(10) == false, "Starting call with ID = 10 again - rejected", passed, failed);
    check(callControlNetwork.startCall(11) == false, "Starting call with ID = 11 - rejected", passed, failed);
    check(callControlNetwork.startCall(12) == true, "Starting call with ID = 12", passed, failed);
    check(callControlNetwork.endCall(10) == true, "Ending call with ID = 10", passed, failed);
    check(callControlNetwork.endCall(10) == false, "Ending call with ID = 10 again - rejected", passed, failed);
    check(callControlNetwork.startCall(11) == false, "Starting call with ID = 11 again - rejected", passed, failed);
    check(callControlNetwork.endCall(12) == true, "Ending call with ID = 12", passed, failed);
    check(callControlNetwork.startCall(11) == true, "Starting call with ID = 11 for third time", passed, failed);
    check(callControlNetwork.endCall(11) == true, "Ending call with ID = 11", passed, failed);
    check(callControlNetwork.startCall(11) == false, "Starting call with ID = 11 for the fourth time - rejected", passed, failed);
    check(callControlNetwork.getCallCount() == 3, "Call amount after all operations stays the same", passed, failed);
}

int main()
{
    int passed = 0;
    int failed = 0;

    testUser(passed, failed);
    testCall(passed, failed);
    testNetworkUserManagement(passed, failed);
    testNetworkCallCreation(passed, failed);
    testNetworkCallControl(passed, failed);

    std::cout << "\nPassed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    if (failed == 0)
    {
        return 0;
    }

    return 1;
}