// Juho Koponen, Nov 2019
// All rights reserved.

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct packageInfo
{
	std::string name;
	std::string description;
	std::vector<std::string> dependencies;
	std::vector<std::string> reverseDependencies;
};

// METHOD PROTOTYPES
bool BCreatePackageList(std::vector<packageInfo>&);
void OrganizePackageList(std::vector<packageInfo>&);
void QuicksortByName(std::vector<packageInfo>&, int, int);
int QSPartition(std::vector<packageInfo>&, int, int);
void CreateHTMLIndex(std::vector<packageInfo>&);
void CreateHTMLPackages(std::vector<packageInfo>&);


int main()
{
	std::vector<packageInfo> packageList;

	if (BCreatePackageList(packageList))
	{
		OrganizePackageList(packageList);
		QuicksortByName(packageList, 0, packageList.size() - 1);

		CreateHTMLIndex(packageList);
		CreateHTMLPackages(packageList);
	}
	std::cout << "Program finished!" << std::endl;

	return 0;
}


// METHODS

bool BCreatePackageList(std::vector<packageInfo>& packageList)
{
	bool isCreated = false;

	std::ifstream ifs("/var/lib/dpkg/status");
	if (!ifs.is_open())
	{
		std::cerr << "Error! Could not open file.";
		exit(1);
	}
	else
	{
		std::string keywordPackage = "Package:";
		std::string keywordDepends = "Depends:";
		std::string keywordDescription = "Description:";


		packageInfo newPackage;
		bool isFirst = true;

		for (std::string line; getline(ifs, line);)
		{
			if (line.compare(0, keywordPackage.size(), keywordPackage) == 0)
			{
				// Add the package to the list (vector) when finding a new package name. Ignore adding when finding the first package name.
				if (isFirst)
				{
					isFirst = false;
				}
				else
				{
					packageList.push_back(newPackage);
					newPackage.name.clear();
					newPackage.description.clear();
					newPackage.dependencies.clear();
				}

				newPackage.name = line.substr(keywordPackage.size() + 1, line.size() - (keywordPackage.size() + 1));
			}
			else if (line.compare(0, keywordDepends.size(), keywordDepends) == 0)
			{
				std::size_t i = keywordDepends.length() + 1;
				std::string excludedCharacters = " ,|(\n";

				while (i < line.length())
				{
					// Multiple dependencies are separated with ',' or with '|'.
					// Dependencies are not found inside parenthesis: ( ).

					if (line[i] == ' ' || line[i] == ',' || line[i] == '|')
					{
						i++;
					}
					else if (line[i] == '(')
					{
						i = line.find_first_of(')', i) + 1;
					}
					else
					{
						newPackage.dependencies.push_back(line.substr(i, line.find_first_of(excludedCharacters, i) - i));
						i = line.find_first_of(excludedCharacters, i);
					}
				}
			}
			else if (line.compare(0, keywordDescription.size(), keywordDescription) == 0)
			{
				newPackage.description = line.substr(keywordDescription.size() + 1, line.size() - (keywordDescription.size() + 1));
			}
		}
		isCreated = true;
	}
	ifs.close();
	return isCreated;
}

void OrganizePackageList(std::vector<packageInfo>& packageList)
{
	// Organize package list and dependencies:
		// delete multiple identical dependencies,
		// assign reverse dependencies.

	for (std::size_t i = 0; i < packageList.size(); i++)
	{
		for (std::size_t j = 0; j < packageList[i].dependencies.size(); j++)
		{
			// Compare for identical dependencies.
			for (std::size_t k = j + 1; k < packageList[i].dependencies.size(); k++)
			{
				if (packageList[i].dependencies[j] == packageList[i].dependencies[k])
				{
					std::vector<std::string>::iterator iter;
					iter = packageList[i].dependencies.begin() + k;
					packageList[i].dependencies.erase(iter);
					k--;
				}
			}

			// Assign reverse dependencies.
			for (int l = 0; l < packageList.size(); l++)
			{
				if (packageList[l].name == packageList[i].dependencies[j])
				{
					packageList[l].reverseDependencies.push_back(packageList[i].name);
				}
			}
		}
	}
}



// Quicksort:
void QuicksortByName(std::vector<packageInfo>& arr, int low, int high)
{
	if (low < high)
	{
		int pIndex = QSPartition(arr, low, high);
		QuicksortByName(arr, low, pIndex - 1);
		QuicksortByName(arr, pIndex + 1, high);
	}
}

int QSPartition(std::vector<packageInfo>& arr, int low, int high)
{
	std::string pivot = arr[high].name;
	int i = (low);

	for (int j = low; j < high; j++)
	{
		if (arr[j].name < pivot)
		{
			std::swap(arr[i], arr[j]);
			i++;
		}
	}
	std::swap(arr[i], arr[high]);
	return i;
}

// HTML creation:
void CreateHTMLIndex(std::vector<packageInfo>& packageList)
{
	std::ofstream ofs("index.html");
	if (!ofs.is_open())
	{
		std::cerr << "Error! Could not write HTML file.";
		exit(1);
	}
	else
	{
		ofs << "<!DOCTYPE html>\n";
		ofs << "<html>\n";

		ofs << "<p>" << "Packages:" << "</p>";

		for (int i = 0; i < packageList.size(); i++)
		{
			ofs << "<a href=\"" << packageList[i].name << ".html\">" << packageList[i].name << "</a>" << "<br>";
		}

		ofs << "</html>";
	}
	ofs.close();
}

void CreateHTMLPackages(std::vector<packageInfo>& packageList)
{
	for (int i = 0; i < packageList.size(); i++)
	{
		std::ofstream ofs(packageList[i].name + ".html");
		if (!ofs.is_open())
		{
			std::cerr << "Error! Could not Could not write HTML file.";
			exit(1);
		}
		else
		{
			ofs << "<!DOCTYPE html>\n";
			ofs << "<html>\n";

			ofs << "<p>" << "Package: " << packageList[i].name << "</p>";

			ofs << "<p>" << "Description:" << "</p>";
			ofs << "<p>" << packageList[i].description << "</p>";

			ofs << "<p>" << "Dependencies:" << "</p>";
			for (int j = 0; j < packageList[i].dependencies.size(); j++)
			{
				ofs << "<a href=\"" << packageList[i].dependencies[j] << ".html\">" << packageList[i].dependencies[j] << "</a>" << "<br>";
			}

			ofs << "<p>" << "Reverse-dependencies:" << "</p>";
			for (int k = 0; k < packageList[i].reverseDependencies.size(); k++)
			{
				ofs << "<a href=\"" << packageList[i].reverseDependencies[k] << ".html\">" << packageList[i].reverseDependencies[k] << "</a>" << "<br>";
			}

			ofs << "</html>";
		}
		ofs.close();
	}
}
