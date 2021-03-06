#include "game.h"
#include "floatfann.h"
#include <stdlib.h>
#include <string.h>

static void
nn_fann_create_network(game_t* game, nn_t* nn, int num_input_neurons, int num_hidden_neurons, int num_output_neurons, number_t learning_rate)
{
  nn->learning_rate = learning_rate;

  struct fann* private = fann_create_standard(3, num_input_neurons, num_hidden_neurons, num_output_neurons);
  fann_set_training_algorithm(private, FANN_TRAIN_INCREMENTAL);
  fann_set_learning_rate(private, learning_rate);

  //fann_set_activation_function_hidden(private, FANN_SIGMOID_STEPWISE);
  fann_set_activation_function_hidden(private, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(private, FANN_LINEAR);

  nn->_private_data = private;
}


static nn_training_data_t*
nn_fann_create_training(nn_t* nn, int num_data, int num_input, int num_output)
{
  nn_training_data_t* train = calloc(1, sizeof(nn_training_data_t));

  train->_private_data = fann_create_train(num_data,
					   num_input,
					   num_output);
  return train;
}


static void
nn_fann_set_training_input_data(nn_training_data_t* train, int sample_index, int neuron_index, number_t value)
{
  struct fann_train_data* private = train->_private_data;
  (*private->input)[(sample_index*private->num_input)+neuron_index] = value;
}


static void
nn_fann_set_training_output_data(nn_training_data_t* train, int sample_index, int neuron_index, number_t value)
{
  struct fann_train_data* private = train->_private_data;
  (*private->output)[(sample_index*private->num_output)+neuron_index] = value;
}


static void
nn_fann_train(nn_t* nn, nn_training_data_t* train, int num_epochs)
{
  struct fann* private_nn = nn->_private_data;
  struct fann_train_data* private_train = train->_private_data;
  fann_train_on_data(private_nn, private_train, num_epochs, 0, 0.1);
}

static number_t
nn_fann_test(struct nn* nn, nn_training_data_t* train)
{
  struct fann_train_data* private_train = train->_private_data;

  number_t error = 0;
  number_t* input = *private_train->input;
  for (int i = 0; i < private_train->num_data; i++) {
    const number_t* q = nn->run(nn, (input_state_t*)input);
    printf("%d: %f -> %f\n", i, (*private_train->output)[i], q[0]);
    input += private_train->num_input;
  }
  printf("\n");
  return error;
}

static const number_t*
nn_fann_run(nn_t* nn, input_state_t* state)
{
  struct fann* private_nn = nn->_private_data;
  return fann_run(private_nn, (void*)state);
}


static void
nn_fann_dump_train(nn_training_data_t* train)
{
  return;
#if 0
#ifdef GAME_ACTION_OUTPUTS
  for (int i = 0; i < train->num_data; i++) {
    number_t* input = &(*train->input)[i*train->num_input];
    number_t* output = &(*train->output)[i*train->num_output];
    printf("player:%2d,%2d  cheese:%2d,%2d pit:%2d,%2d %s: % 4.4f  %s % 4.4f %c\n",
	   (int)(input[0]*GAME_MAP_SIZE_X),
	   (int)(input[1]*GAME_MAP_SIZE_Y),
	   (int)(input[2]*GAME_MAP_SIZE_X),
	   (int)(input[3]*GAME_MAP_SIZE_Y),
	   (int)(input[4]*GAME_MAP_SIZE_X),
	   (int)(input[5]*GAME_MAP_SIZE_Y),
	   state_action_names[0],
	   output[0],
	   state_action_names[1],
	   output[1],
	   output[0] <  output[1] ? '*' : ' ');
  }
#else
  for (int i = 0; i < train->num_data; i++) {
    number_t* input = &(*train->input)[i*train->num_input];
    number_t* output = &(*train->output)[i*train->num_output];
    printf("player:%2d,%2d  cheese:%2d,%2d pit:%2d,%2d  Q: % 4.4f Action: %s:\n",
      (int)(input[1]*GAME_MAP_SIZE_X),
      (int)(input[2]*GAME_MAP_SIZE_Y),
      (int)(input[3]*GAME_MAP_SIZE_X),
      (int)(input[4]*GAME_MAP_SIZE_Y),
      (int)(input[5]*GAME_MAP_SIZE_X),
      (int)(input[6]*GAME_MAP_SIZE_Y),
      output[0],
      state_action_names[(int)input[0]]);
  }
#endif
#endif
}


static void
nn_fann_load(nn_t* nn, const char* filename)
{
  nn->_private_data = fann_create_from_file(filename);
}


static void
nn_fann_save(nn_t* nn, const char* filename)
{
  struct fann* private_nn = nn->_private_data;
  fann_save(private_nn, filename);
}


static struct nn*
nn_fann_clone(struct nn* nn)
{
  nn_t* clone = calloc(1, sizeof(nn_t));
  memcpy(clone, nn, sizeof(nn_t));
  clone->_private_data = fann_copy(nn->_private_data);
  return clone;
}


static void
nn_fann_destroy(struct nn* nn)
{
  fann_destroy(nn->_private_data);
  free(nn);
}



nn_t*
nn_fann_construct(void)
{
  nn_t* nn = calloc(1, sizeof(nn_t));
  nn->create_network = nn_fann_create_network;
  nn->create_training = nn_fann_create_training;
  nn->set_training_input_data = nn_fann_set_training_input_data;
  nn->set_training_output_data = nn_fann_set_training_output_data;
  nn->train = nn_fann_train;
  nn->run = nn_fann_run;
  nn->load = nn_fann_load;
  nn->save = nn_fann_save;
  nn->dump_train = nn_fann_dump_train;
  nn->clone = nn_fann_clone;
  nn->destroy = nn_fann_destroy;
  nn->test = nn_fann_test;
  return nn;
}
