#pragma once
#include "DynamicVariableGroup.h"

template<class T>
class DynamicVariableManager
{
public:
	DynamicVariableManager(const std::initializer_list<std::string>& dynamicVariableGroupNames)
	{
		for (const auto& name : dynamicVariableGroupNames)
		{
			mNameToVariableGroup.insert({ name, std::make_shared<DynamicVariableGroup<T>>(name) });
		}
		
		// Run the loop on a separate thread, so that we do not stall the main thread
		// when we are waiting for the user
		mThread = std::thread(&DynamicVariableManager::Loop, this);
	}

	~DynamicVariableManager()
	{
		mThread.join();
	}
	
	// Thread-safe
	std::shared_ptr<DynamicVariableGroup<T>> GetGroup(const std::string& groupName)
	{
		std::lock_guard lockGuard(mMutex);

		// Make sure that the group actually exists
		assert(mNameToVariableGroup.find(dynamicVariableGroupName) != mNameToVariableGroup.end());

		return mNameToVariableGroup[groupName];
	}


private:
	// Thread-safe
	void Loop()
	{
		while (true)
		{
			// Get the name of the variable group that the user wants to update
			if (auto nameOptional = GetNameFromUser())
			{
				std::shared_ptr<DynamicVariableGroup<float>> variableGroup;
				
				// Only lock guard the part where we are accessing
				// our data
				{
					std::lock_guard lockGuard(mMutex);
					variableGroup = mNameToVariableGroup[*nameOptional];
				}
				

				variableGroup->Loop();
			}
			else
			{
				// The user does not want to udpate any variables groups
				return;
			}
		}
	}

private:
	// Thread-safe
	std::optional<std::string> GetNameFromUser()
	{
		std::string name;
		do
		{
			LOG("Type the name of the variable group that you want to update (or \"q\" to quit):" << std::endl);
			CIN(name);

			if (name == "q")
			{
				// The user did not want to update any variable groups, so return no name
				return {};
			}

			// Continue to ask the question until the user provides a valid name
		} while (!IsNameValid(name));

		return name;
	}
	// Thread-safe
	bool IsNameValid(const std::string& name)
	{
		std::lock_guard lockGuard(mMutex);
		return mNameToVariableGroup.find(name) != mNameToVariableGroup.end();
	}
private:
	std::thread mThread;
	std::mutex mMutex;

	std::unordered_map<std::string, std::shared_ptr<DynamicVariableGroup<T>>> mNameToVariableGroup;
};