// NanoTask.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <NanoTask.hpp>

const char* taskFormat = "%s Task\n";

bool bRemoveTask1 = false;

int main()
{
	NanoTask::TaskManager mgr;

	auto tsk1 = std::make_unique<NanoTask::Task>(
		std::chrono::seconds(1),
		[](std::string str) {
			printf(taskFormat, str.c_str());
		},
		"1 Second"
	);

	auto tsk2 = std::make_unique<NanoTask::Task>(
		std::chrono::seconds(5),
		[](std::string str) {
			printf(taskFormat, str.c_str());
		},
		"5 Second"
	);

	auto tsk3 = std::make_unique<NanoTask::Task>(
		std::chrono::seconds(10),
		[](std::string str) {
			printf(taskFormat, str.c_str());
		},
		"10 Second"
	);

	auto tsk4 = std::make_unique<NanoTask::Task>(
		std::chrono::seconds(15),
		[](std::string str) {
			printf(taskFormat, str.c_str());
			bRemoveTask1 = true;
		},
		"15 Second"
	);

	mgr.Add("1Sec", tsk1);
	mgr.Add(tsk2);
	mgr.Add("10Sec", tsk3);
	mgr.Add(tsk4);

	bool bTask1Removed = false;

	while (/* Some Loop Logic */ true)
	{
		mgr.Update();

		if (bTask1Removed == false && bRemoveTask1)
		{
			mgr.Remove("1Sec");
			bTask1Removed = true;
		}
	}
}