#pragma once
#include "Logger.h"

namespace Tests
{
	class LogTestUtils
	{
	public:
        void static LogTestName(const std::string& name)
        {
            LOG_INFO("\n**********************************************\n%s\n**********************************************", name.c_str());
        }

        void static LogTestResult(bool correct)
        {
            std::string text;
            if (correct)
            {
                text = "CORRECT";
            }
            else
            {
                text = "ERROR";
            }

            LOG_INFO("\n**********************************************\n%s\n**********************************************\n\n", text.c_str());
        }
	};
}
