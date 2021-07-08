/*
 * dromajo_manycore.h
 * API definitons for Dromajo/BlackParrot to interact with the Manycore
 */

#ifndef _MANYCORE_H
#define _MANYCORE_H 1

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <queue>
#include <map>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of credits for requests from the host to the manycore
// NOTE: Keep this in sync with the hardware implementation
#define MAX_CREDITS 32
#define RW_FAIL_CODE 0xFFFFFFFF

// All 128-bit FIFOs can have a maximum of N elements each
#ifndef FIFO_MAX_ELEMENTS
#define FIFO_MAX_ELEMENTS 32
#endif

// NOTE: The host does not respond to the manycore

typedef union mc_pkt_t {
  // 128-bit packet sent/received over the DPI
  __int128_t pkt128;
  // 4 32-bit packets to send to dromajo manycore FIFOs
  uint32_t pkt32[4];
} mc_pkt_t;

typedef enum mc_fifo_type_t {
  FIFO_HOST_TO_MC_REQ = 0,
  FIFO_MC_TO_HOST_REQ = 1,
  FIFO_MC_TO_HOST_RESP = 2
} mc_fifo_type_t;

// All FIFOs are N element FIFOs. These aim to mimic the SIPOs and PISOs in the manycore bridge hardware
typedef struct mc_fifo_t {
  // 4 32-bit FIFOs to capture the 128-bit manycore packets
  std::vector<std::queue<uint32_t>> fifo;
  // Is this FIFO initialized?
  bool init;
  // Is the FIFO full? If all FIFOs are full, the 128-bit FIFO output is considered "valid".
  // A "valid" output can then be transmitted to the manycore or read by BP
  std::vector<bool> full;
  // Number of credits for FIFOs on the BP->MC request path; Unused otherwise
  int credits;
} mc_fifo_t;

// Define the 128-bit FIFOs
extern mc_fifo_t *host_to_mc_req_fifo;
extern mc_fifo_t *mc_to_host_req_fifo;
extern mc_fifo_t *mc_to_host_resp_fifo;

// FIFO ID --> FIFO Index map
static std::map<uint32_t, int> index_map {{0x0, 0}, {0x4, 1}, {0x8, 2}, {0xc, 3}};

////////////////////////// RISC-V Host Interface //////////////////////////

/*
 * Initializes all FIFOs
 * @param[in] type - Type of the FIFO to initialize
 */
void mc_fifo_init(mc_fifo_t *fifo);

/*
 * Returns if the FIFO is full
 * A 0 indicates the FIFO is not full
 * A 1 indicates the FIFO is full
 * @param[in] type - Type of FIFO to query
 * @param[in] _full - Set to true if you need the fullness of a specific 32-bit FIFO
 * @param[in] fifo_id - Chooses which 32-bit FIFO's fullness to return
 * @returns FIFO full status
 */
bool mc_is_fifo_full(mc_fifo_type_t type, bool _full = false, uint32_t fifo_id = 0x0);

/*
 * Returns if the FIFO is empty
 * A 0 indicates the FIFO is not empty
 * A 1 indicates the FIFO is empty
 * @param[in] type - Type of FIFO to query
 * @param[in] _full - Set to true if you need the fullness of a specific 32-bit FIFO
 * @param[in] fifo_id - Chooses which 32-bit FIFO's fullness to return
 * @returns FIFO empty status
 */
bool mc_is_fifo_empty(mc_fifo_type_t type, bool _empty = false, uint32_t fifo_id = 0x0);

/*
 * Write to the desired FIFO
 * @param[in] type - Type of FIFO to write
 * @param[in] fifo_id - Chooses which 32-bit FIFO to write
 * @param[in] val - The 32-bit value to be written
 * @return FIFO write status
 *  false --> Write was unsuccesful
 *  true --> Write was successful
 */
bool mc_fifo_write(mc_fifo_type_t type, uint32_t fifo_id, uint32_t val);

/*
 * Read from the desired FIFO
 * @param[in] offset - Chooses which 32-bit FIFO to read
 * @param[in] val - Pointer to store the 32-bit read value 
 * @param[in] type - Type of FIFO to read
 * @return FIFO read status
 *  false --> Read was unsuccesful
 *  true --> Read was successful
 */
bool mc_fifo_read(mc_fifo_type_t type, uint32_t offset, uint32_t *val);

/*
 * Get the credits for the desired FIFO
 * Only BP will request the manycore for credits
 * @params[in] type - Type of FIFO requesting credits
 * @returns the remaining FIFO credits
 */
int mc_fifo_get_credits(mc_fifo_type_t type);

#ifdef __cplusplus
}  // extern C
#endif

#endif
