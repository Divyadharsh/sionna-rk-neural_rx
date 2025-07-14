# capture
#set(PHY_NEURAL_RX_CAPTURE_SRC
	#${OPENAIR1_DIR}/PHY/NR_TRANSPORT/nr_ulsch_demodulation.c
  #nr_neural_rx_capture.c
  #)

#add_library(neural_rx_capture MODULE ${PHY_NEURAL_RX_CAPTURE_SRC})
#target_link_libraries(neural_rx_capture PRIVATE pthread)

#add_dependencies(nr-softmodem neural_rx_capture)
#add_dependencies(nr-uesoftmodem neural_rx_capture)
#add_dependencies(neural_rx_capture generate_T)

# START marker-plugin-cmake
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIT})

# Set your OpenAirInterface root and paths if not set already
set(OPENAIR1_DIR ${CMAKE_SOURCE_DIR}/openair1)
set(NFAPI_DIR ${CMAKE_SOURCE_DIR}/nfapi/open-nFAPI/nfapi/)

set(PHY_NEURAL_RX_CAPTURE_SRC
  ${OPENAIR1_DIR}/PHY/NR_TRANSPORT/nr_ulsch_demodulation.c
  ${CMAKE_CURRENT_SOURCE_DIR}/nr_neural_rx_capture.c
)

add_library(neural_rx_capture MODULE ${PHY_NEURAL_RX_CAPTURE_SRC})

# Add include directories so headers can be found
target_include_directories(neural_rx_capture PRIVATE
  ${OPENAIR1_DIR}
  ${OPENAIR1_DIR}/PHY
  ${OPENAIR1_DIR}/PHY/NR_TRANSPORT
  ${OPENAIR1_DIR}/PHY/NR_REFSIG
  ${OPENAIR1_DIR}/PHY/NR_ESTIMATION
  ${OPENAIR1_DIR}/PHY/TOOLS
  ${CMAKE_SOURCE_DIR}/common/utils
  ${CMAKE_SOURCE_DIR}/common/utils/nr
  ${NFAPI_DIR}/public_inc  # for nfapi_nr_interface_scf.h
)


# Link pthread and any OAI libraries your code depends on
target_link_libraries(neural_rx_capture PRIVATE pthread)

# Add any dependencies your module has
add_dependencies(nr-softmodem neural_rx_capture)
add_dependencies(nr-uesoftmodem neural_rx_capture)
add_dependencies(neural_rx_capture generate_T)

