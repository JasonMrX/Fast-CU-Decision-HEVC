#include "TLibCommon/basic_tools_YS.h"

/////////////////////////////////
/// basic_tools
///////////////////////////////////

int WriteData(string fileName, cv::Mat& matData)
{
	int retVal = 0;

	// ���ļ�  
	ofstream outFile;//(fileName.c_str(), ios_base::out);  //���½��򸲸Ƿ�ʽд�� 
	outFile.open(fileName.c_str());
	if (!outFile.is_open())
	{
		cout << "���ļ�ʧ��" << endl;
		retVal = -1;
		return (retVal);
	}

	// �������Ƿ�Ϊ��  
	if (matData.empty())
	{
		cout << "����Ϊ��" << endl;
		retVal = 1;
		return (retVal);
	}

	// д������  
	for (int r = 0; r < matData.rows; r++)
	{
		for (int c = 0; c < matData.cols; c++)
		{
			double data = matData.at<double>(r, c);  //��ȡ���ݣ�at<type> - type �Ǿ���Ԫ�صľ������ݸ�ʽ  
			outFile << data << "\t";   //ÿ�������� tab ����  
		}
		outFile << endl;  //����  
	}
	outFile.close();
	return (retVal);
}

int LoadData(string fileName, cv::Mat& matData, int matRows = 0, int matCols = 0, int matChns = 0)
{
	int retVal = 0;

	// ���ļ�  
	ifstream inFile(fileName.c_str(), ios_base::in);
	if (!inFile.is_open())
	{
		cout << "��ȡ�ļ�ʧ��" << endl;
		retVal = -1;
		return (retVal);
	}

	// ��������  
	istream_iterator<float> begin(inFile);    //�� float ��ʽȡ�ļ�����������ʼָ��  
	istream_iterator<float> end;          //ȡ�ļ�������ֹλ��  
	vector<float> inData(begin, end);      //���ļ����ݱ����� std::vector ��  
	cv::Mat tmpMat = cv::Mat(inData);       //�������� std::vector ת��Ϊ cv::Mat  

	// ����������д���  
	//copy(vec.begin(),vec.end(),ostream_iterator<double>(cout,"\t"));   

	// ����趨�ľ���ߴ��ͨ����  
	size_t dataLength = inData.size();
	//1.ͨ����  
	if (matChns == 0)
	{
		matChns = 1;
	}
	//2.������  
	if (matRows != 0 && matCols == 0)
	{
		matCols = dataLength / matChns / matRows;
	}
	else if (matCols != 0 && matRows == 0)
	{
		matRows = dataLength / matChns / matCols;
	}
	else if (matCols == 0 && matRows == 0)
	{
		matRows = dataLength / matChns;
		matCols = 1;
	}
	//3.�����ܳ���  
	if (dataLength != (matRows * matCols * matChns))
	{
		cout << "��������ݳ��� ������ �趨�ľ���ߴ���ͨ����Ҫ�󣬽���Ĭ�Ϸ�ʽ�������" << endl;
		retVal = 1;
		matChns = 1;
		matRows = dataLength;
	}

	// ���ļ����ݱ������������  
	matData = tmpMat.reshape(matChns, matRows).clone();

	return (retVal);
}


//file
template <typename T> void create3dFileArray(T***& array, int d_1, int d_2, int d_3){
	array = new T**[d_1];
	for (int i = 0; i < d_1; i++){
		array[i] = new T*[d_2];
		for (int j = 0; j < d_2; j++){
			array[i][j] = new T[d_3];
		}
	}
}

template <typename T> void create2dFileArray(T**& array, int d_1, int d_2){
	array = new T*[d_1];
	for (int i = 0; i < d_1; i++){
		array[i] = new T[d_2];
	}
}
//3D
template <typename T> void create3dArray(T***& array, int d_1, int d_2, int d_3){
	array = new T**[d_1];
	for (int i = 0; i < d_1; i++){
		array[i] = new T*[d_2];
		for (int j = 0; j < d_2; j++){
			array[i][j] = new T[d_3];
			for (int k = 0; k < d_3; k++)array[i][j][k] = 0;
		}
	}
}
template void create3dArray(double***& array, int d_1, int d_2, int d_3);
template void create3dArray(bool***& array, int d_1, int d_2, int d_3);

template <typename T> void clear3dArray(T***& array, int d_1, int d_2, int d_3){
	for (int i = 0; i < d_1; i++){
		for (int j = 0; j < d_2; j++){
			for (int k = 0; k < d_3; k++) array[i][j][k] = 0;
		}
	}
}

template <typename T> void delete3dArray(T***& array, int d_1, int d_2, int d_3){
	for (int i = 0; i < d_1; i++){
		for (int j = 0; j < d_2; j++)delete[] array[i][j];
		delete[] array[i];
	}
	delete[] array;
	array = NULL;
}
template void delete3dArray(double***& array, int d_1, int d_2, int d_3);
template void delete3dArray(bool***& array, int d_1, int d_2, int d_3);


// 2D
template <typename T> void create2dArray(T**& array, int d_1, int d_2){
	array = new T*[d_1];
	for (int i = 0; i < d_1; i++){
		array[i] = new T[d_2];
		for (int j = 0; j < d_2; j++) array[i][j] = 0;

	}
}

template <typename T> void clear2dArray(T**& array, int d_1, int d_2){
	for (int i = 0; i < d_1; i++){
		for (int j = 0; j < d_2; j++)array[i][j] = 0;
	}
}
template void clear2dArray(double**& array, int d_1, int d_2);
template void clear2dArray(bool**& array, int d_1, int d_2);

template <typename T> void delete2dArray(T**& array, int d_1, int d_2){
	for (int i = 0; i < d_1; i++)delete[] array[i];
	delete[] array;
	array = NULL;
}



template <typename T> void print3dArray(T*** array, int d_1, int d_2, int d_3){
	for (int i = 0; i < d_1; i++){
		for (int j = 0; j < d_2; j++){
			for (int k = 0; k < d_3; k++)cout << array[i][j][k];

			cout << endl;
		}
		cout << endl;
	}
}
template void print3dArray(double*** array, int d_1, int d_2, int d_3);
template void print3dArray(bool*** array, int d_1, int d_2, int d_3);
template <typename T> void setArray(T*& array, int d_1, T value){
	for (int i = 0; i < d_1; i++)array[i] = value;
}


Int findPos(Double *data, Int right, Double tgt){
	Int left = 0;
	while (left <= right) {
		Int mid = left + (right - left) / 2;
		if ((mid - 1 >= 0 && data[mid] > tgt && data[mid - 1] <= tgt) || (mid == 0 && data[mid] > tgt)){
			return mid;
		}
		else if (data[mid] >= tgt){
			right = mid - 1;
		}
		else{
			left = mid + 1;
		}
	}
	return left;
}