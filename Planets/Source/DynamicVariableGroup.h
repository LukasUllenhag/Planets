#pragma once
#include <thread>
#include "Console/Log.h"
#include "Console/ConsoleInput.h"
#include <optional>
#include "Keyboard.h"
#include "CustomException.h"

template<class T>
class DynamicVariableGroup
{
public:
	DynamicVariableGroup(const std::string& filename)
		:
		mFilename(filename)
	{
		std::ifstream file = OpenFile(FILE_PATH + mFilename + FILE_EXTENSION);

		std::vector<std::string> strings;
		// Divide the file into a vector of strings
		std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), 
			std::back_inserter(strings));

		// Partition the vector so that the numbers come first. We use a stable partition, since
		// we want to perserve the relative order of the numbers and strings.
		auto beginningOfNames = std::stable_partition(strings.begin(), strings.end(),
			[](const std::string& string)
			{
				// If the string only contains digits, dots and minus signs, it is considered
				// to be a number
				return std::all_of(string.begin(), string.end(),
					[](const char c)
					{
						return std::isdigit(c) || c == '.' || c == '-';
					});
			});

		// Loop through all of the strings that contain numbers and convert them to type "T"
		std::transform(strings.begin(), beginningOfNames, std::back_inserter(mVariables),
			[](const std::string& string)
			{
				std::stringstream stringStream(string);
				T value = (T)0;
				stringStream >> value;
				return value;
			});

		// Loop through all of the variable names
		std::for_each(beginningOfNames, strings.end(),
			[this, index = 0](const std::string& name) mutable
			{
				// The first occurring name is the name of the first occurring
				// variable, i.e., the variable at index 0,
				// the second occurring name is the name of the second occurring
				// variable, i.e., the variable at index 1, etc. 
				mNameToVariableIndex.insert({ name, index });
				mVariableIndexToName.insert({ index, name });
				++index;
			});
	}
	~DynamicVariableGroup()
	{
		if (DoesUserWantToSave())
		{
			SaveVariables();
		}
	}

	// Thread-safe
	void Loop()
	{
		while (true)
		{
			// Get the name of the variable that the user wants to update
			if (auto nameOptional = GetNameFromUser())
			{
				SetVariableToUpdateName(*nameOptional);
			}
			else
			{
				// The user does not want to udpate any variables, so reset the variable to 
				// update to nullptr
				SetVariableToUpdate(nullptr);
				return;
			}

			SetVariableToUpdate(GetVariable(mVariableToUpdateName));

			UpdateValueConsole();
		}
	}

	// Thread-safe
	// The only way to read the variables from the outside is through this 
	// method, "GetVariables" and "UseVariables", which are all thread-safe
	T Get(int index)
	{
		std::lock_guard lockGuard(mMutex);
		assert(index >= 0 && index < mVariables.size());
		return mVariables[index];
	}
	// Thread-safe
	// The only way to read the variables from the outside is through this 
	// method, "Get" and "UseVariables", which are all thread-safe
	std::vector<T> GetVariables()
	{
		std::lock_guard lockGuard(mMutex);
		return mVariables;
	}
	// Thread-safe
	// The only way to read the variables from the outside is through this 
	// method, "Get" and "GetVariables", which are all thread-safe
	void UseVariables(std::function<void(const T*, size_t)> function)
	{
		std::lock_guard lockGuard(mMutex);
		function(&mVariables.front(), mVariables.size());
	}
	// Thread-safe
	// Updates the value of the current variable to update (if there is one),
	// through keyboard input
	bool UpdateValueKeyboard(float deltaTime)
	{
		std::lock_guard lockGuard(mMutex);
		bool valueIsUpdated = false;

		// Only proceed if there is a variable the user wants to update
		if (mVariableToUpdate)
		{
			UpdateUpdateSpeedKeyboard(deltaTime);

			if (Keyboard::KeyIsPressed(GLFW_KEY_LEFT))
			{
				*mVariableToUpdate -= deltaTime * mUpdateSpeed;
				valueIsUpdated = true;
			}
			else if (Keyboard::KeyIsPressed(GLFW_KEY_RIGHT))
			{
				*mVariableToUpdate += deltaTime * mUpdateSpeed;
				valueIsUpdated = true;
			}

			if (valueIsUpdated)
			{
				LOG(mVariableToUpdateName + " = " << *mVariableToUpdate << std::endl);
			}
		}

		return valueIsUpdated;
	}
private:
	// Updates the update speed through keyboard input. The method does not
	// lock the mutex, and should only get called by UpdateValueKeyboard().
	void UpdateUpdateSpeedKeyboard(float deltaTime)
	{
		float speedIsUpdated = false;
		if (Keyboard::KeyIsPressed(GLFW_KEY_UP))
		{
			mUpdateSpeed += deltaTime * mUpdateAcceleration;
			speedIsUpdated = true;
		}
		else if (Keyboard::KeyIsPressed(GLFW_KEY_DOWN))
		{
			mUpdateSpeed -= deltaTime * mUpdateAcceleration;
			speedIsUpdated = true;
		}
		if (speedIsUpdated)
		{
			// The speed should only be positive
			mUpdateSpeed = std::max(mUpdateSpeed, 0.0f);
			LOG("Update speed = " << mUpdateSpeed << std::endl);
		}
	}
	std::ifstream OpenFile(const std::string& filePath) const
	{
		std::ifstream file;
		// Make the file stream throw exceptions. Note that we are not setting the
		// failbit, since we could fail when we are converting the whole file into
		// a vector of strings (see constructor), due to the possibilty of extracting 
		// an empty string.
		file.exceptions(std::ifstream::badbit);
		file.open(filePath);

		// Since we did not set the failbit, we have to manually check for a fail
		if (!file.good())
		{
			throw CREATE_CUSTOM_EXCEPTION("Failed to open: " + filePath);
		}
	
		return file;
	}
	// Thread-safe
	void UpdateValueConsole()
	{
		while (true)
		{
			{
				std::lock_guard lockGuard(mMutex);
				LOG("Type new value (or \"q\" to quit) for \"" + mVariableToUpdateName  + "\" = " << *mVariableToUpdate << ": " << std::endl);
			}

			std::string stringValue;
			CIN(stringValue);
			if (stringValue == "q")
			{
				SetVariableToUpdate(nullptr);
				return;
			}
			else
			{
				std::istringstream stringStream(stringValue);
				T value;
				stringStream >> value;

				std::lock_guard lockGuard(mMutex);
				// We can not use SetVariableToUpdate(), since it sets the pointer
				// and not the actual value
				*mVariableToUpdate = value;
				LOG(mVariableToUpdateName + " = " << *mVariableToUpdate << std::endl);
			}
		}
	}
	// Thread-safe
	std::optional<std::string> GetNameFromUser()
	{
		std::string name;
		do
		{
			LOG("Type the name of the variable that you want to update (or \"q\" to quit):" << std::endl);
			CIN(name);

			if (name == "q")
			{
				// The user did not want to update any variables, so return no name
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
		return mNameToVariableIndex.find(name) != mNameToVariableIndex.end();
	}
	// Thread-safe
	T* GetVariable(const std::string& name)
	{
		std::lock_guard lockGuard(mMutex);
		return &mVariables[mNameToVariableIndex.at(name)];
	}
	// Thread-safe
	void SetVariableToUpdate(T* value)
	{
		std::lock_guard lockGuard(mMutex);
		mVariableToUpdate = value;
	}
	// Thread-safe
	void SetVariableToUpdateName(const std::string& name)
	{
		std::lock_guard lockGuard(mMutex);
		mVariableToUpdateName = name;
	}

	bool DoesUserWantToSave() const
	{
		// Make sure that this is the only thread that logs or uses "CIN" during the duration of this scope.
		// We do not want the output, "LOG", to get detached from the input, "CIN".
		std::scoped_lock scopedLock(gLogMutex, gConsoleInputMutex);

		std::string input;
		// Loop until user inputs "y" or "n"
		while (true)
		{
			LOG("Do you want to save the updated variables for " + mFilename + " ? (y/n):" << std::endl);
			CIN(input);

			if (input == "y")
			{
				return true;
			}
			else if (input == "n")
			{
				return false;
			}
		}
		
	}
	void SaveVariables() const
	{
		std::string filename;
		{
			// Make sure that this is the only thread that logs or uses "CIN" during the duration of this scope.
			// We do not want the output, "LOG", to get detached from the input, "CIN".
			std::scoped_lock scopedLock(gLogMutex, gConsoleInputMutex);

			LOG("Save file as (type \"o\" to override current file): ");
			CIN(filename);
			if (filename == "o")
			{
				filename = mFilename;
			}
		}
		std::ofstream file;
		file.exceptions(std::ios::badbit | std::ios::failbit);
		file.open(FILE_PATH + filename + FILE_EXTENSION);

		// Reconstruct the file
		for (size_t i = 0; i < mVariables.size(); ++i)
		{
			file << mVariableIndexToName.at(int(i)) << " " << mVariables[i] << std::endl;
		}
	}
private:
	std::string mFilename;

	std::mutex mMutex;

	T* mVariableToUpdate = nullptr;
	std::string mVariableToUpdateName;

	std::vector<T> mVariables;
	std::unordered_map<std::string, int> mNameToVariableIndex;
	std::unordered_map<int, std::string> mVariableIndexToName;

	float mUpdateSpeed = 5.0f;
	float mUpdateAcceleration = 1.0f;
	inline static const std::string FILE_PATH = "Source/DynamicVariableFiles/";
	inline static const std::string FILE_EXTENSION = ".txt";
};