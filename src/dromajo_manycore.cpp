/*
 * API definitions for Dromajo to interact with the HammerBlade Manycore
 */

#include "dromajo_manycore.h"
#include <iostream>

// Helper functions
bool get_fifo_empty(mc_fifo_t *fifo, bool _empty = false, uint32_t fifo_id = 0x0);
uint32_t get_fifo_size(mc_fifo_t *fifo, uint32_t fifo_id = 0x0);
bool get_fifo_full(mc_fifo_t *fifo, bool _full = false, uint32_t fifo_id = 0x0);
void set_fifo_full(mc_fifo_t *fifo, uint32_t fifo_id);

int get_fifo_credits(mc_fifo_t *fifo);
bool fifo_write(mc_fifo_t *fifo, uint32_t fifo_id, uint32_t val);
bool fifo_read(mc_fifo_t *fifo, uint32_t fifo_id, uint32_t *val);

/*
 * Initializes all FIFOs
 * This function 
 *    sets the full vector of the FIFO to all zeros (all FIFOs empty at the start)
 *    defines the 4 32-bit FIFOs
 *    sets the credits used to the minimum value
 *    sets the init field to True
 * @param[in] fifo - FIFO to initialize
 */
void mc_fifo_init(mc_fifo_t *fifo) {
  for (int i = 0; i < 4; i++) {
    fifo->fifo.push_back(std::queue<uint32_t>());
    fifo->full.push_back(false);
  }
  fifo->credits = 0;
  fifo->init = true;
}

/*
 * Returns the state of the FIFO
 * A 0 indicates the FIFO is empty
 * A 1 indicates the FIFO is full
 * @param[in] type - Type of FIFO to query
 * @param[in] _full - Set to true if you need the fullness of a specific 32-bit FIFO
 * @param[in] fifo_id - Chooses which 32-bit FIFO's fullness to return
 * @returns FIFO full status
 */
bool mc_is_fifo_full(mc_fifo_type_t type, bool _full, uint32_t fifo_id) {
  bool full= false;
  switch (type) {
    case FIFO_HOST_TO_MC_REQ:
      full = get_fifo_full(host_to_mc_req_fifo, _full, fifo_id);
    break;
    case FIFO_MC_TO_HOST_REQ:
      full = get_fifo_full(mc_to_host_req_fifo, _full, fifo_id);
    break;
    case FIFO_MC_TO_HOST_RESP:
      full = get_fifo_full(mc_to_host_resp_fifo, _full, fifo_id);
    break;
    default:
    {
      printf("Undefined/Unknown FIFO type!\nExiting...\n");
      exit(-1);
    }
    break;
  }
  return full;
}

/*
 * Returns if the FIFO is empty
 * A 0 indicates the FIFO is not empty
 * A 1 indicates the FIFO is empty
 * @param[in] type - Type of FIFO to query
 * @param[in] _full - Set to true if you need the fullness of a specific 32-bit FIFO
 * @param[in] fifo_id - Chooses which 32-bit FIFO's fullness to return
 * @returns FIFO empty status
 */
bool mc_is_fifo_empty(mc_fifo_type_t type, bool _empty, uint32_t fifo_id) {
  bool empty = false;
  switch (type) {
    case FIFO_HOST_TO_MC_REQ:
      empty = get_fifo_empty(host_to_mc_req_fifo, _empty, fifo_id);
    break;
    case FIFO_MC_TO_HOST_REQ:
      empty = get_fifo_empty(mc_to_host_req_fifo, _empty, fifo_id);
    break;
    case FIFO_MC_TO_HOST_RESP:
      empty = get_fifo_empty(mc_to_host_resp_fifo, _empty, fifo_id);
    break;
    default:
    {
      printf("Undefined/Unknown FIFO type!\nExiting...\n");
      exit(-1);
    }
    break;
  }
  return empty;
}

/*
 * Write to the desired FIFO
 * @param[in] type - Type of FIFO to write
 * @param[in] offset - Chooses which 32-bit FIFO to write
 * @param[in] val - The 32-bit value to be written
 * @return FIFO write status
 *  false --> Write was unsuccesful
 *  true --> Write was successful
 */
bool mc_fifo_write(mc_fifo_type_t type, uint32_t fifo_id, uint32_t val) {
  bool fifo_write_success = false;
  switch (type) {
    case FIFO_HOST_TO_MC_REQ:
      fifo_write_success = fifo_write(host_to_mc_req_fifo, fifo_id, val);
    break;
    case FIFO_MC_TO_HOST_REQ:
      fifo_write_success = fifo_write(mc_to_host_req_fifo, fifo_id, val);
    break;
    case FIFO_MC_TO_HOST_RESP:
      fifo_write_success = fifo_write(mc_to_host_resp_fifo, fifo_id, val);
    break;
    default:
    {
      printf("Undefined/Unknown FIFO type!\nExiting...\n");
      exit(-1);
    }
    break;
  }
  return fifo_write_success;
}

/*
 * Read from the desired FIFO
 * @param[in] offset - Chooses which 32-bit FIFO to read
 * @param[in] val - Pointer to store the 32-bit read value 
 * @param[in] type - Type of FIFO to read
 * @return FIFO read status
 *  false --> Read was unsuccesful
 *  true --> Read was successful
 */
bool mc_fifo_read(mc_fifo_type_t type, uint32_t fifo_id, uint32_t *val) {
  bool fifo_read_success = false;
  switch (type) {
    case FIFO_HOST_TO_MC_REQ:
      fifo_read_success = fifo_read(host_to_mc_req_fifo, fifo_id, val);
    break;
    case FIFO_MC_TO_HOST_REQ:
      fifo_read_success = fifo_read(mc_to_host_req_fifo, fifo_id, val);
    break;
    case FIFO_MC_TO_HOST_RESP:
      fifo_read_success = fifo_read(mc_to_host_resp_fifo, fifo_id, val);
    break;
    default:
    {
      printf("Undefined/Unknown FIFO type!\nExiting...\n");
      exit(-1);
    }
    break;
  }
  return fifo_read_success;
}

/*
 * Get the credits for the desired FIFO
 * Only BP will request the manycore for credits
 * @params[in] type - Type of FIFO requesting credits
 * @returns the remaining FIFO credits
 */
int mc_fifo_get_credits(mc_fifo_type_t type) {
  int credits = 0;
  switch (type) {
    case FIFO_HOST_TO_MC_REQ:
      credits = get_fifo_credits(host_to_mc_req_fifo);
    break;
    case FIFO_MC_TO_HOST_REQ:
    case FIFO_MC_TO_HOST_RESP:
      printf("Manycore will not request credits from the host\n");
    break;
    default:
    {
      printf("Undefined/Unknown FIFO type!\nExiting...\n");
      exit(-1);
    }
    break;
  }
  return credits;
}

/*
 * Returns if a given 32-bit or 128-bit FIFO is empty
 * @params[in] fifo - 128-bit FIFO to check emptiness
 * @params[in] _empty - Set to true if you need the emptiness of a given 32-bit FIFO
 * @params[in] fifo_id - Chooses which 32-bit FIFO's emptiness to return
 * @returns 32-bit/128-bit FIFO emptiness
 */
bool get_fifo_empty(mc_fifo_t *fifo, bool _empty, uint32_t fifo_id) {
  if (_empty)
    return fifo->fifo[index_map[fifo_id]].empty();
  else {
    std::vector<std::queue<uint32_t>>::iterator it;
    bool is_empty = false;
    for(it = fifo->fifo.begin(); it != fifo->fifo.end(); ++it) {
      is_empty |= it->empty();
    }
    return is_empty;
  }
}

/*
 * Returns the number of elements in a given 32-bit FIFO
 * @params[in] fifo - 128-bit FIFO to query size
 * @params[in] fifo_id - Chooses which 32-bit FIFO's size is queried
 * @returns number of elements in the FIFO
 */
uint32_t get_fifo_size(mc_fifo_t *fifo, uint32_t fifo_id) {
  return fifo->fifo[index_map[fifo_id]].size();
}

/*
 * Returns if a given 32-bit or 128-bit FIFO is full
 * @params[in] fifo - 128-bit FIFO to check fullness
 * @params[in] _empty - Set to true if you need the fullness of a given 32-bit FIFO
 * @params[in] fifo_id - Chooses which 32-bit FIFO's fullness to return
 * @returns 32-bit/128-bit FIFO fullness
 */
bool get_fifo_full(mc_fifo_t *fifo, bool _full, uint32_t fifo_id) {
  if (_full)
    return fifo->full[index_map[fifo_id]];
  else {
    std::vector<bool>::iterator it;
    bool is_full = true;
    for(it = fifo->full.begin(); it != fifo->full.end(); ++it)
      is_full &= *it;
    return is_full; 
  }
}

/*
 * Sets the fullness of a given 32-bit FIFO
 * @params[in] fifo - 128-bit FIFO to set fullness
 * @params[in] fifo_id - Chooses which 32-bit FIFO's fullness to set
 */
void set_fifo_full(mc_fifo_t *fifo, uint32_t fifo_id) {
  uint32_t fifo_size = get_fifo_size(fifo, fifo_id);
  if ((fifo_size == FIFO_MAX_ELEMENTS))
    fifo->full[index_map[fifo_id]] = true;
  else
    fifo->full[index_map[fifo_id]] = false;
}

/*
 * Returns the number of credits available in the manycore for host requests
 *    This field is set by the DPI function
 *    This is useful only for the BP->MC request path
 * @params[in] fifo - FIFO who credits we want to query
 * @returns the remaining FIFO credits
 */
int get_fifo_credits(mc_fifo_t *fifo) {
  return fifo->credits;
}

/*
 * Write to the desired 32-bit FIFO
 * @params[in] fifo - 128-bit FIFO to write
 * @params[in] fifo_id - 32-bit FIFO to write
 * @params[in] val - 32-bit data to write
 * @returns true if the write was successful else false
 */
bool fifo_write(mc_fifo_t *fifo, uint32_t fifo_id, uint32_t val) {
  // Check if the FIFO is initialized
  bool is_fifo_init = fifo->init;
  if (!is_fifo_init) {
    printf("FATAL: FIFO uninitialized! Initialize FIFO before proceeding.\nExiting...\n");
    exit(-1);
  }

  // Check if the FIFO is full
  bool is_fifo_full = get_fifo_full(fifo, true, fifo_id);
  if (is_fifo_full)
    return false;

  // Write to the FIFO
  fifo->fifo[index_map[fifo_id]].push(val);
  // Set the corresponding full bit
  set_fifo_full(fifo, fifo_id);
  return true;
}

/*
 * Read from the desired 32-bit FIFO
 * @params[in] fifo - 128-bit FIFO to read
 * @params[in] fifo_id - 32-bit FIFO to read
 * @params[in] val - pointer to the 32-bit read value
 * @returns true if the read was successful else false
 */
bool fifo_read(mc_fifo_t *fifo, uint32_t fifo_id, uint32_t *val) {
  // Check if the FIFO is initialized
  bool is_fifo_init = fifo->init;
  if (!is_fifo_init) {
    printf("FATAL: FIFO uninitialized! Initialize FIFO before proceeding.\nExiting...\n");
    exit(-1);
  }

  // Check if the FIFO is empty
  bool is_fifo_empty = get_fifo_empty(fifo, true, fifo_id);
  if (is_fifo_empty)
    return false;

  // Read the head of the FIFO
  *val = fifo->fifo[index_map[fifo_id]].front();
  // Pop the head of the FIFO
  fifo->fifo[index_map[fifo_id]].pop();
  // Set the corresponding 32-bit FIFO's full bit 
  // If the FIFO is empty, this will set the full bit to zero
  set_fifo_full(fifo, fifo_id);
  return true;
}
