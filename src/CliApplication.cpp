#include "CliApplication.h"

CliApplication::CliApplication(Network &network, std::istream &input, std::ostream &output) : mNetwork(network), mInput(input), mOutput(output) {}

int CliApplication::run()
{
    while (true)
    {
        printMenu(mOutput);

        const auto option = readMenuOption(mInput);

        if (!option.has_value())
        {
            mOutput << "Invalid option\n";
            continue;
        }

        if (*option == MenuOption::Exit)
        {
            return 0;
        }

        handleOption(*option);
    }
}

void CliApplication::handleOption(MenuOption option)
{
    switch (option)
    {
    case MenuOption::AddUser:
        handleAddUser(mNetwork, mInput, mOutput);
        break;

    case MenuOption::RemoveUser:
        handleRemoveUser(mNetwork, mInput, mOutput);
        break;

    case MenuOption::CreateCall:
        handleCreateCall(mNetwork, mInput, mOutput);
        break;

    case MenuOption::StartCall:
        handleStartCall(mNetwork, mInput, mOutput);
        break;

    case MenuOption::EndCall:
        handleEndCall(mNetwork, mInput, mOutput);
        break;

    case MenuOption::ListUsers:
        handleListUsers(mNetwork, mOutput);
        break;

    case MenuOption::ListCalls:
        handleListCalls(mNetwork, mOutput);
        break;

    case MenuOption::Save:
        handleSave(mNetwork, mInput, mOutput);
        break;

    case MenuOption::Load:
        handleLoad(mNetwork, mInput, mOutput);
        break;

    case MenuOption::Exit:
        break;
    }
}