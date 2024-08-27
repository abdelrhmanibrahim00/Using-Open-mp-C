

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>

using namespace std;

#define OUTPUTFILE "C:\\Users\\pc\\Desktop\\lab1_bfnl1\\lab1_bfnl1\\result.txt"

#define INPUTFILE "C:\\Users\\pc\\Desktop\\lab1_bfnl1\\lab1_bfnl1\\f3.txt"

#define THREADS 4


struct Student
{
	string  stringField;
	int          intField;
	double       doubleField;

	Student()
	{
		stringField = "";
		intField = 0;
		doubleField = 0.0;
	}

	Student(string a, int b, double c)
	{
		stringField = a;
		intField = b;
		doubleField = c;
	}
};

bool ResultComputation(const Student& student) {
	if (student.doubleField > 6 && student.intField > 200)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

class ResultMonitor {
public:
	Student students[25];
	int n;
	int intSum;
	double floatSum;

	ResultMonitor()
	{
		n = 0;
		intSum = 0;
		floatSum = 0.0;
	}


	int getSize() const
	{
		return n;
	}

	int getIntSum() const {
		return intSum;
	}

	double getFloatSum() const {
		return floatSum;
	}


	Student get(int h) const
	{
		if (h >= 0 && h < n)
		{
			return students[h];
		}
		else
		{
			// Handle the out-of-bounds case appropriately, maybe throw an exception or return a default Student object.
			// For now, return a default-constructed Student object.
			return Student();
		}
	}


	void AddItem(Student d)
	{
		#pragma omp critical
		{
			// Find the correct position to insert the new student based on doubleField
			int insertIndex = 0;
			while (insertIndex < n && students[insertIndex].doubleField > d.doubleField)
			{
				insertIndex++;
			}
			// Shift elements to make space for the new student
			for (int i = n; i > insertIndex; i--)
			{
				students[i] = students[i - 1];
			}
			// Insert the new student at the correct position
			students[insertIndex] = d;
			n++;
		}

		
	}


	
};



string stringFieldName, intFieldName, doubleFieldName;

int dataElementsCount;

int threadDataSize;







vector<Student> ReadFromFile(const string& filename) {
	vector<Student> students;

	try {
		ifstream reader(filename);
		string line;

		while (getline(reader, line)) {
			size_t delimiterPos = line.find(';');

			if (delimiterPos != string::npos) {
				string stringField = line.substr(0, delimiterPos);
				int intField = stoi(line.substr(delimiterPos + 1, line.find(';', delimiterPos + 1) - delimiterPos - 1));
				double doubleField = stod(line.substr(line.find(';', delimiterPos + 1) + 1));

				Student student(stringField, intField, doubleField);
				students.push_back(student);
			}
		}
	}
	catch (const exception& e) {
		cerr << "Exception: " << e.what() << endl;
	}

	return students;
}

void readFile(vector<Student*>& threadDataArray)
{
	ifstream input(INPUTFILE);

	input >> stringFieldName;
	input >> intFieldName;
	input >> doubleFieldName;
	input >> dataElementsCount;

	threadDataSize = ceil((double)dataElementsCount / THREADS);
	cout << " Data element = " << dataElementsCount << endl;
	cout << "Thread Data Size = " << threadDataSize << endl;


	int line = 0;
	for (int i = 0; i < THREADS; i++)
	{
		Student* threadData = new Student[threadDataSize];

		for (int j = 0; j < threadDataSize; j++)
		{
			string stringField;
			int intField;
			double doubleField;

			input >> stringField >> intField >> doubleField;

			if (line < dataElementsCount)
				threadData[j] = Student(stringField, intField, doubleField);
			else
				threadData[j] = Student();

			line++;
		}

		threadDataArray.push_back(threadData);
	}
	input.close();
}

void writeData(vector<Student*> threadDataArray, string text)
{
	ofstream textfile;
	textfile.open(text);
	int line = 0;
	textfile << stringFieldName << "\t" << intFieldName << "\t" << doubleFieldName << "\r\n";
	for (int i = 0; i < THREADS; i++)
	{
		textfile << endl << "** Array" << i << " **" << endl;
		for (int j = 0; j < threadDataSize; j++)
		{
			line++;

			if (threadDataArray[i][j].stringField != "")
			{
				cout.precision(2);
				textfile << j << ") " << threadDataArray[i][j].stringField << "\t" << threadDataArray[i][j].intField << "\t" << fixed << threadDataArray[i][j].doubleField << "\r\n";
			}

			if (line == dataElementsCount)
				break;
		}
	}
	textfile.close();
	cout << endl;
}





void Result(ResultMonitor& r, const std::string& filePath) {
	std::ofstream outputFile(filePath);

	if (outputFile.is_open()) {
		int size = r.getSize(); // Get the number of elements in the ResultMonitor

		// Print title
		outputFile << "The list of the accepted students to get the internship" << std::endl;

		// Print table header
		outputFile << "Number \tName \tCredits \tGPA" << std::endl;

		// Print student information in a table format
		for (int i = 0; i < size; ++i) {
			Student student = r.get(i); // Retrieve a student using the get(i) method
			// Print student information to the file in table format
			outputFile << i+1 << "\t" << student.stringField << "\t" << student.intField << "\t" << student.doubleField << std::endl;
		}

		// Print total sums
		outputFile << "Total Sum of Int Fields: " << r.getIntSum() << std::endl;
		outputFile << "Total Sum of Double Fields: " << r.getFloatSum() << std::endl;

		outputFile.close();
		std::cout << "Data written to the file: " << filePath << std::endl;
	}
	else {
		std::cerr << "Error: Unable to open the file for writing." << std::endl;
	}
}






void ParallelStudentProcessing(vector<Student>& studentList, ResultMonitor& r, int numThreads)
{
	omp_set_num_threads(numThreads);

	int dataSize = studentList.size();
	int chunkSize = dataSize / numThreads;
	int remainder = dataSize % numThreads;

	int intSum = 0;
	double floatSum = 0.0;

#pragma omp parallel reduction(+:intSum) reduction(+:floatSum)
	{
		int threadId = omp_get_thread_num();
		int startIdx = threadId * chunkSize + min(threadId, remainder);
		int endIdx = (threadId + 1) * chunkSize + min(threadId + 1, remainder);

		for (int i = startIdx; i < endIdx; ++i)
		{
			Student& student = studentList[i];
			if (ResultComputation(student))
			{
				intSum += student.intField;
				floatSum += student.doubleField;
				r.AddItem(student);
			}
		}
	}

	
	r.intSum += intSum;
	r.floatSum += floatSum;
}



int main(int argc, char* argv[])
{
	ResultMonitor r;
	vector<Student> Read;
	Read=ReadFromFile(INPUTFILE);
	std::cout << "Number of elements in Read: " << Read.size() << std::endl;
	
	ParallelStudentProcessing(Read, r, THREADS);
	
	cout << "Size of ResultMonitor: " << r.getSize() << endl;
	Result(r, OUTPUTFILE);
	

	
	return 0;
};




