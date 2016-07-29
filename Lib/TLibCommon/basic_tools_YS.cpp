#include "TLibCommon/basic_tools_YS.h"

/////////////////////////////////
/// basic_tools
///////////////////////////////////

int WriteData(string fileName, cv::Mat& matData)
{
	int retVal = 0;

	// 打开文件  
	ofstream outFile;//(fileName.c_str(), ios_base::out);  //按新建或覆盖方式写入 
	outFile.open(fileName.c_str());
	if (!outFile.is_open())
	{
		cout << "打开文件失败" << endl;
		retVal = -1;
		return (retVal);
	}

	// 检查矩阵是否为空  
	if (matData.empty())
	{
		cout << "矩阵为空" << endl;
		retVal = 1;
		return (retVal);
	}

	// 写入数据  
	for (int r = 0; r < matData.rows; r++)
	{
		for (int c = 0; c < matData.cols; c++)
		{
			double data = matData.at<double>(r, c);  //读取数据，at<type> - type 是矩阵元素的具体数据格式  
			outFile << data << "\t";   //每列数据用 tab 隔开  
		}
		outFile << endl;  //换行  
	}
	outFile.close();
	return (retVal);
}

int LoadData(string fileName, cv::Mat& matData, int matRows = 0, int matCols = 0, int matChns = 0)
{
	int retVal = 0;

	// 打开文件  
	ifstream inFile(fileName.c_str(), ios_base::in);
	if (!inFile.is_open())
	{
		cout << "读取文件失败" << endl;
		retVal = -1;
		return (retVal);
	}

	// 载入数据  
	istream_iterator<float> begin(inFile);    //按 float 格式取文件数据流的起始指针  
	istream_iterator<float> end;          //取文件流的终止位置  
	vector<float> inData(begin, end);      //将文件数据保存至 std::vector 中  
	cv::Mat tmpMat = cv::Mat(inData);       //将数据由 std::vector 转换为 cv::Mat  

	// 输出到命令行窗口  
	//copy(vec.begin(),vec.end(),ostream_iterator<double>(cout,"\t"));   

	// 检查设定的矩阵尺寸和通道数  
	size_t dataLength = inData.size();
	//1.通道数  
	if (matChns == 0)
	{
		matChns = 1;
	}
	//2.行列数  
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
	//3.数据总长度  
	if (dataLength != (matRows * matCols * matChns))
	{
		cout << "读入的数据长度 不满足 设定的矩阵尺寸与通道数要求，将按默认方式输出矩阵！" << endl;
		retVal = 1;
		matChns = 1;
		matRows = dataLength;
	}

	// 将文件数据保存至输出矩阵  
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