#include "FileEventLogger.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

TEST(FileEventLoggerTests, WritesMessagesToFile)
{
    const std::filesystem::path filePath = std::filesystem::temp_directory_path() / "telecom_file_logger_test.log";

    std::filesystem::remove(filePath);

    {
        FileEventLogger logger(filePath.string());

        logger.log("First event");
        logger.log("Second event");
    } // logger i jego ofstream zostają zniszczone, plik jest zamknięty

    std::string firstLine;
    std::string secondLine;

    {
        std::ifstream file(filePath);

        ASSERT_TRUE(file.is_open());
        ASSERT_TRUE(std::getline(file, firstLine));
        ASSERT_TRUE(std::getline(file, secondLine));
    } // ifstream zostaje zniszczony, plik jest zamknięty

    EXPECT_EQ(firstLine, "[EVENT] First event");
    EXPECT_EQ(secondLine, "[EVENT] Second event");

    std::filesystem::remove(filePath);
}

TEST(FileEventLoggerTests, ThrowsWhenFileCannotBeOpened)
{
    const std::string invalidPath = "/definitely/nonexistent/directory/events.log";

    EXPECT_THROW(FileEventLogger logger(invalidPath), std::runtime_error);
}