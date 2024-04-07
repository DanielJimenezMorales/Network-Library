#pragma once
#include <sstream>

#include "Logger.h"

namespace Tests
{
	class LogTestUtils
	{
	public:
        void static LogTestName(const std::string& name)
        {
            std::stringstream ss;
            ss << "\n**********************************************\n" << name << "\n" << "**********************************************";
            Common::LOG_INFO(ss.str());
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

            std::stringstream ss;
            ss << "\n**********************************************\n" << text << "\n" << "**********************************************\n\n";
            Common::LOG_INFO(ss.str());
        }
	};
}
