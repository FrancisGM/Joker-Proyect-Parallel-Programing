#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <cmath>
#include <string>

using Matrix = std::vector<std::vector<double>>;

std::atomic<int> totalThreads(0);
std::chrono::duration<double> parallel_time(0);
std::chrono::duration<double> sequential_time(0);

void printMatrix(const Matrix &matrix){
    for (const auto &row : matrix){
        for (int element : row){
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }
}

Matrix generateMatrix(int dimensions, double value){
    Matrix result;
    result.resize(dimensions, std::vector<double>(dimensions));
    for(int i = 0; i<dimensions;i++){
        for (int j=0; j<dimensions; j++){
            result[i][j]=value;
        }
    }
    return result;
}

void addMatrices(const Matrix &matrixA, const Matrix &matrixB, Matrix &result, int startRow, int endRow){
    for (int i = startRow; i < endRow; ++i){
        for (size_t j = 0; j < matrixA[i].size(); ++j){
            result[i][j] = matrixA[i][j] + matrixB[i][j];
        }
    }
    totalThreads++;
}

void subtractMatrices(const Matrix &matrixA, const Matrix &matrixB, Matrix &result,int startRow, int endRow){
    for (int i = startRow; i < endRow; ++i){
        for (size_t j = 0; j < matrixA[i].size(); ++j){
            result[i][j] = matrixA[i][j] - matrixB[i][j];
        }
    }
    totalThreads++;
}

void multiplyMatrices(const Matrix &matrixA, const Matrix &matrixB, Matrix &result, int startRow, int endRow){
    for (int i = startRow; i < endRow; ++i){
        for (size_t j = 0; j < matrixB[0].size(); ++j){
            result[i][j] = 0;
            for (size_t k = 0; k < matrixA[i].size(); ++k){
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
    totalThreads++;
}

void parallelMatrixOperation(const Matrix &matrixA, const Matrix &matrixB, Matrix &result, int numThreads, std::string operation){
    double rowsPerThreadDouble = matrixA.size() / numThreads;
    int rowsPerThread = static_cast<int>(floor(rowsPerThreadDouble));
    std::vector<std::thread> threads;
    if (operation == "suma"){
        auto parallel_startTime = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numThreads - 1; ++i){
            threads.emplace_back(addMatrices, std::ref(matrixA), std::ref(matrixB), std::ref(result), i * rowsPerThread, (i + 1) * rowsPerThread);
        }
        threads.emplace_back(addMatrices, std::ref(matrixA), std::ref(matrixB), std::ref(result), (numThreads - 1) * rowsPerThread, matrixA.size());
        for (auto &thread : threads){
            thread.join();
        }
        parallel_time = std::chrono::high_resolution_clock::now() - parallel_startTime;
    }
    else{
        auto parallel_startTime = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numThreads - 1; ++i){
            threads.emplace_back(subtractMatrices, std::ref(matrixA), std::ref(matrixB), std::ref(result), i * rowsPerThread, (i + 1) * rowsPerThread);
        }
        threads.emplace_back(subtractMatrices, std::ref(matrixA), std::ref(matrixB), std::ref(result), (numThreads - 1) * rowsPerThread, matrixA.size());
        for (auto &thread : threads){
            thread.join();
        }
        parallel_time = std::chrono::high_resolution_clock::now() - parallel_startTime;
    }
}

void parallelMatrixMultiplication(const Matrix &matrixA, const Matrix &matrixB, Matrix &result, int numThreads){
    double rowsPerThreadDouble = matrixA.size() / numThreads;
    int rowsPerThread = static_cast<int>(floor(rowsPerThreadDouble));
    std::vector<std::thread> threads;

    auto parallel_startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads - 1; ++i){
        threads.emplace_back(multiplyMatrices, std::ref(matrixA), std::ref(matrixB), std::ref(result),
                             i * rowsPerThread, (i + 1) * rowsPerThread);
    }

    threads.emplace_back(multiplyMatrices, std::ref(matrixA), std::ref(matrixB), std::ref(result), (numThreads - 1) * rowsPerThread, matrixA.size());

    for (auto &thread : threads) thread.join();
    parallel_time = std::chrono::high_resolution_clock::now() - parallel_startTime;
}

void getInputMatrix(Matrix &matrix, const std::string &matrixName){
    std::cout << "Ingrese las dimensiones de la matriz " << matrixName << " (filas y columnas): ";
    int rows, cols;
    std::cin >> rows >> cols;

    std::cout << "Ingrese los valores de la matriz " << matrixName << ":" << std::endl;
    matrix.resize(rows, std::vector<double>(cols));

    for (int i = 0; i < rows; ++i){
        for (int j = 0; j < cols; ++j){
            std::cout << matrixName << "[" << i << "][" << j << "]: ";
            std::cin >> matrix[i][j];
        }
    }
}

void opcMatrixMultiplication(){
    Matrix matrixA, matrixB, result;

    getInputMatrix(matrixA, "A");
    getInputMatrix(matrixB, "B");
    //matrixA = generateMatrix(100,2);
    //matrixB= generateMatrix(100,2);

    Matrix matrixA_2 = matrixA;
    Matrix matrixB_2 = matrixB;

    if (matrixA[0].size() != matrixB.size()){
        std::cerr << "Operacion no valida." << std::endl;
        return;
    }

    result.resize(matrixA.size(), std::vector<double>(matrixB[0].size(), 0));

    Matrix result_2 = result;

    int numThreads = std::thread::hardware_concurrency();
    while(numThreads > static_cast<int>(matrixA.size())){
        numThreads--;
    }

    auto sequential_startTime = std::chrono::high_resolution_clock::now();
    multiplyMatrices(matrixA_2, matrixB_2, result_2, 0, matrixA_2.size());
    sequential_time = std::chrono::high_resolution_clock::now() - sequential_startTime;

    parallelMatrixMultiplication(matrixA, matrixB, result, numThreads);

    std::cout << "\nMatrix A:" << std::endl;
    printMatrix(matrixA);

    std::cout << "\nMatrix B:" << std::endl;
    printMatrix(matrixB);

    std::cout << "\nResult Matrix:" << std::endl;
    printMatrix(result);

    std::cout << "\nTotal de hilos utilizados: " << totalThreads << std::endl;
    std::cout << "Tiempo de ejecucion secuencial: " << sequential_time.count() * 1000 << " ms" << std::endl;
    std::cout << "Tiempo de ejecucion paralelo: " << parallel_time.count() * 1000 << " ms" << std::endl;
    std::cout << "Speedup: " << sequential_time / parallel_time << std::endl;
    std::cout << "Eficiencia: " << 100*(sequential_time / parallel_time)/numThreads << std::endl;
}

void opcMatrixOperation(std::string operation)
{
    Matrix matrixA, matrixB, result;

    getInputMatrix(matrixA, "A");
    getInputMatrix(matrixB, "B");
    //matrixA = generateMatrix(300,2);
    //matrixB= generateMatrix(300,2);
    Matrix matrixA_2 = matrixA;
    Matrix matrixB_2 = matrixB;


    if (matrixA.size() != matrixB.size() || matrixA[0].size() != matrixB[0].size()){
        std::cerr << "Las matrices no tienen dimensiones compatibles para la " << operation << "." << std::endl;
        return;
    }

    result.resize(matrixA.size(), std::vector<double>(matrixA[0].size(), 0));

    Matrix result_2 = result;

    int numThreads = std::thread::hardware_concurrency();
    while(numThreads > static_cast<int>(matrixA.size())){
        numThreads--;
    }

    parallelMatrixOperation(matrixA, matrixB, result, numThreads, operation);

    if(operation =="suma"){
        auto sequential_startTime = std::chrono::high_resolution_clock::now();
        addMatrices(matrixA_2,matrixB_2,result_2,0,matrixA.size());
        sequential_time = std::chrono::high_resolution_clock::now() - sequential_startTime;
    }else{
        auto sequential_startTime = std::chrono::high_resolution_clock::now();
        subtractMatrices(matrixA_2,matrixB_2,result_2,0,matrixA.size());
        sequential_time = std::chrono::high_resolution_clock::now() - sequential_startTime;
    }

    std::cout << "\nMatrix A:" << std::endl;
    printMatrix(matrixA);

    std::cout << "\nMatrix B:" << std::endl;
    printMatrix(matrixB);

    std::cout << "\nResult Matrix (Sum):" << std::endl;
    printMatrix(result);

    std::cout << "\nTotal de hilos utilizados: " << totalThreads << std::endl;
    std::cout << "\nTotal de hilos utilizados: " << totalThreads << std::endl;
    std::cout << "Tiempo de ejecucion secuencial: " << sequential_time.count() * 1000 << " ms" << std::endl;
    std::cout << "Tiempo de ejecucion paralelo: " << parallel_time.count() * 1000 << " ms" << std::endl;
    std::cout << "Speedup: " << sequential_time / parallel_time << std::endl;
    std::cout << "Eficiencia: " << 100*(sequential_time / parallel_time)/numThreads << std::endl;
}

int main()
{

    char opcion;
    do{
        std::cout << "Calculadora de matrices:" << std::endl;
        std::cout << "1. Suma" << std::endl;
        std::cout << "2. Resta" << std::endl;
        std::cout << "3. Multiplicacion" << std::endl;
        std::cout << "4. Salir" << std::endl;
        std::cout << "Ingrese la opcion deseada: ";
        std::cin >> opcion;
        switch (opcion)
        {
        case '1':
        {
            std::cout << "----------------------------" << std::endl;
            opcMatrixOperation("suma");
            std::cout << "----------------------------" << std::endl;
            break;
        }
        case '2':
        {
            std::cout << "----------------------------" << std::endl;
            opcMatrixOperation("resta");
            std::cout << "----------------------------" << std::endl;
            break;
        }
        case '3':
        {
            std::cout << "----------------------------" << std::endl;
            opcMatrixMultiplication();
            std::cout << "----------------------------" << std::endl;
            break;
        }
        case '4':
            std::cout << "Saliendo de la aplicación. ¡Hasta luego!" << std::endl;
            return 0;
        default:
            std::cout << "Opcion no válida. Por favor, ingrese una opcion válida." << std::endl;
        }
        totalThreads = 0;
        // Preguntar si desea volver al menú principal o salir
        std::cout << "Desea volver al Menu Principal? (S/N): ";
        std::cin >> opcion;

    } while (opcion == 'S' || opcion == 's');
    std::cout << "Jose Francisco Gutierrez Mudarra 21200131" << std::endl;
    return 0;
}
