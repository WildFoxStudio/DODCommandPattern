#include <iostream>
#include <queue>
#include <memory>
#include <string_view>

/* Example by Stanislav Kirichenko */

// The class that will receive the messages
class ReceiverInterface
{
    public:
    virtual void Print(std::string_view message) = 0;
};


class Receiver : public ReceiverInterface
{
public:
    void Print(std::string_view message) override { std::cout << message; }
};

// Base command structure
struct BaseCommand
{
    BaseCommand() = default;
    void(*CmdFn)(BaseCommand* thisCommand, ReceiverInterface& receiver);
};

// Concrete struct
struct PrintCommand: public BaseCommand
{
    std::string_view Message{};
    PrintCommand(std::string_view message) : Message{message}
    {
        // Setup an anonymous function
        CmdFn = [](BaseCommand* thisCommand, ReceiverInterface& receiver){
            // Cast to this struct
            PrintCommand* cmd = static_cast<PrintCommand*>(thisCommand);
            // Execute
            receiver.Print(cmd->Message);
        };
    }
};

int main() {
#pragma region MainThread
    // Not optimal, good enough for the example: 
    // Use std::prm or custom memory allocator to reduce allocation bottleneck
    auto cmd = std::make_unique<PrintCommand>("Hello world!");
#pragma endregion

    // Use any queue/BUS you like
    std::queue<std::unique_ptr<BaseCommand>> lockFreeQueue{};
    lockFreeQueue.emplace(std::move(cmd));

#pragma region WorkerThread
    Receiver receiver{};
    // Execute incoming events on the other thread
    lockFreeQueue.back()->CmdFn(lockFreeQueue.back().get(), receiver);
#pragma endregion
}
