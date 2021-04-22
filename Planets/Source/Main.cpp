#include "Game.h"
#include "CustomException.h"
#include <optional>
#include "Benchmark/BenchmarkMacros.h"
#include "Console/ErrorLog.h"

int main()
{
    // The status of the execution is 
    // initially set to success
    int executionStatus = EXIT_SUCCESS;

    #if ENABLE_BENCHMARKING
        // Using "std::optional" in order to defer the construction of "benchmarkSession"
        std::optional<benchmark::Session> benchmarkSession;
    #endif

    // Using "std::optional" in order to defer the construction of "game"
    std::optional<Game> game;
    try
    {
        #if ENABLE_BENCHMARKING
            // Creating a benchmark session that exists during the entire lifetime of "game"
            benchmarkSession.emplace("Main");
        #endif  
        game.emplace();
    }
    catch (const CustomException& exception)
    {
        ERROR_LOG(exception.what());
        // Update the execution status
        // to signal failure
        executionStatus = EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        ERROR_LOG(exception.what());
        // Update the execution status
        // to signal failure
        executionStatus = EXIT_FAILURE;
    }
    catch (...)
    {
        ERROR_LOG("Unknown exception");
        // Update the execution status
        // to signal failure
        executionStatus = EXIT_FAILURE;
    }
    
    try
    {
        game->BeginLoop();
    }
    catch (const CustomException& exception)
    {
        ERROR_LOG(exception.what());
        // Update the execution status
        // to signal failure
        executionStatus = EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        ERROR_LOG(exception.what());
        // Update the execution status
        // to signal failure
        executionStatus = EXIT_FAILURE;
    }
    catch (...)
    {
        ERROR_LOG("Unknown exception");
        // Update the execution status
        // to signal failure
        executionStatus = EXIT_FAILURE;
    }

    return executionStatus;
}
