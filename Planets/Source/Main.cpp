#include "Game.h"
#include "CustomException.h"
#include <optional>
#include "Benchmark/BenchmarkMacros.h"
#include "Console/ErrorLog.h"

int main()
{
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
    }
    catch (const std::exception& exception)
    {
        ERROR_LOG(exception.what());
    }
    catch (...)
    {
        ERROR_LOG("Unknown exception");
    }
    
    try
    {
        game->BeginLoop();
    }
    catch (const CustomException& exception)
    {
        ERROR_LOG(exception.what());
    }
    catch (const std::exception& exception)
    {
        ERROR_LOG(exception.what());
    }
    catch (...)
    {
        ERROR_LOG("Unknown exception");
    }

    return 0;
}
