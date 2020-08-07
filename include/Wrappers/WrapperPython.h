#pragma once
#include <iostream>
#ifdef _DEBUG
#undef _DEBUG
#include <torch/extension.h>
#include <THC/THC.h>
#include <ATen/ATen.h>
#if (__linux__)
#include <pybind11/pybind11.h>
#include <torch/csrc/utils/pybind.h>
#endif
#define _DEBUG
#else
#include <torch/extension.h>
#include <THC/THC.h>
#include <ATen/ATen.h>
#if (__linux__)
#include <pybind11/pybind11.h>
#include <torch/csrc/utils/pybind.h>
#endif
#endif

#include "Common.h"
#include "Parser.h"
#include "Decoder.h"
#include "VideoProcessor.h"

class StreamPool {
public:
	int cacheStream(std::string inputFile);
	std::shared_ptr<Parser> getParser(std::string inputFile);
	std::map<std::string, std::shared_ptr<Parser> > getParsers();
	std::shared_ptr<Logger> getLogger();
	int setLogger(std::shared_ptr<Logger> logger);
private:
	std::shared_ptr<Logger> logger;
	std::map<std::string, std::shared_ptr<Parser> > parserArr;
};

class TensorStream {
public:
int addStreamPool(std::shared_ptr<StreamPool> streamPool);
int resetPipeline(std::string inputFile);
	int initPipeline(std::string inputFile, uint8_t maxConsumers, uint8_t cudaDevice, uint8_t decoderBuffer, FrameRateMode frameRate, bool cuda, int threads);
	std::map<std::string, int> getInitializedParams();
	int startProcessing(int cudaDevice = 0);
	std::tuple<at::Tensor, int> getFrame(std::string consumerName, int index, FrameParameters frameParameters);
	at::Tensor getFrameAbsolute(std::vector<int> index, FrameParameters frameParameters);
	void endProcessing();
	void enableLogs(int logsLevel);
	void enableNVTX();
	int dumpFrame(at::Tensor stream, std::string consumerName, FrameParameters frameParameters);
	void skipAnalyzeStage();
	void setTimeout(int timeout);
	int getTimeout();
	int enableBatchOptimization();
private:
	int processingLoop();
	std::shared_ptr<StreamPool> streamPool = nullptr;
	int gopSize = 32;
	std::mutex syncDecoded;
	std::mutex syncRGB;
	std::shared_ptr<Parser> parser;
	std::shared_ptr<Decoder> decoder;
	std::shared_ptr<VideoProcessor> vpp;
	AVPacket* parsed;
	int realTimeDelay = 0;
	double indexToDTSCoeff = 0;
	double DTSToMsCoeff = 0;
	std::pair<int, int> frameRate;
	FrameRateMode frameRateMode;
	bool shouldWork;
	bool skipAnalyze;
	std::vector<std::pair<std::string, AVFrame*> > decodedArr;
	std::vector<std::pair<std::string, AVFrame*> > processedArr;
	std::vector<at::Tensor> tensors;
	std::vector<std::shared_ptr<uint8_t> > processedFrames;
	std::mutex freeSync;
	std::mutex closeSync;
	std::map<std::string, bool> blockingStatuses;
	std::mutex blockingSync;
	std::condition_variable blockingCV;
	std::shared_ptr<Logger> logger;
	uint8_t currentCUDADevice;
	bool _cuda;
};