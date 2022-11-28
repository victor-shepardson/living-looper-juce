// LLModel encapsulates the libtorch parts
#pragma once

#include <torch/torch.h>
#include <torch/script.h>

struct LLModel {

  torch::jit::Module model;

  int sr;
  int block_size;
  int z_per_second;
  int latent_size;
  int n_loops;
  bool loaded;
    
  std::vector<torch::jit::IValue> inputs_rave;
  std::vector<torch::jit::IValue> inputs_empty;

  LLModel() {
    this->loaded=false;
    torch::jit::getProfilingMode() = false;
    c10::InferenceMode guard;
    torch::jit::setGraphExecutorOptimize(true);
    }
    
  void load(const std::string& rave_model_file) {
    std::cout << "LivingLooper: loading \"" <<rave_model_file << "\"" <<std::endl;
    try {
        c10::InferenceMode guard;
        this->model = torch::jit::load(rave_model_file);
        this->model.eval();
        // this->model = torch::jit::optimize_for_inference(this->model);
    }
    catch (const c10::Error& e) {
      // why no error when filename is bad?
        std::cout << e.what();
        std::cout << e.msg();
        std::cout << "error loading the model\n";
        return;
    }

    this->block_size = this->latent_size = this->sr = this->n_loops = -1;

    for (auto const& attr: model.named_attributes()) {
        if (attr.name == "n_loops") {this->n_loops = attr.value.toInt();} 
        if (attr.name == "block_size") {this->block_size = attr.value.toInt();} 
        if (attr.name == "sampling_rate") {this->sr = attr.value.toInt();} 
    }
    // this->z_per_second = this->sr / this->block_size;

    if ((this->block_size<=0) || 
        (this->n_loops<=0) || 
        // (this->latent_size<0) || 
        (this->sr<=0)){
      std::cout << "model load failed" << std::endl;
      return;
    }

    std::cout << "\tnumber of loops: " << this->n_loops << std::endl;
    std::cout << "\tblock size: " << this->block_size << std::endl;
    // std::cout << "\tlatent size: " << this->latent_size << std::endl;
    std::cout << "\tsample rate: " << this->sr << std::endl;

    c10::InferenceMode guard;
    inputs_rave.clear();
    inputs_rave.push_back(torch::IValue(0));
    inputs_rave.push_back(torch::ones({1,1,block_size}));

    this->loaded = true;
  }

  void reset () {
    c10::InferenceMode guard;
    this->model.get_method("reset")(inputs_empty);
  }

  torch::Tensor forward_tensor (float* input, int loop_idx) {
    c10::InferenceMode guard;

    inputs_rave[0] = torch::IValue(loop_idx); 

    inputs_rave[1] = torch::from_blob(
      input, block_size).reshape({1, 1, block_size});

    return this->model(inputs_rave).toTensor();

  }

  void forward (float* input, int loop_idx, float* outBuffer) {
    c10::InferenceMode guard;

    // auto t_start = std::chrono::high_resolution_clock::now();
    const auto y = this->forward_tensor(input, loop_idx);
    // auto elapsed = std::chrono::high_resolution_clock::now() - t_start;
		// std::cout << "model time: " << elapsed.count() * 1e-6 << " ms" << std::endl;

    auto acc = y.accessor<float, 3>();
    for(int j=0; j<acc.size(0); j++) {
      for(int i=0; i<acc.size(2); i++) {
        outBuffer[n_loops*i + j] = acc[j][0][i];
      }
    }
  }

};