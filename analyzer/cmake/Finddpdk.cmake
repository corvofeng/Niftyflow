# Try to find dpdk
#
# Once done, this will define
#
# DPDK_FOUND
# DPDK_INCLUDE_DIR
# DPDK_LIBRARIES


# It's ugly. maybe solve it later.

if(NOT EXISTS "$ENV{DPDK_DIR}")
    message(FATAL_ERROR "Could not find ${Red} DPDK_DIR ${ColourReset} environment variable")
endif(NOT EXISTS "$ENV{DPDK_DIR}")

find_path(DPDK_INCLUDE_DIR rte_config.h
  PATH_SUFFIXES dpdk
  HINTS $ENV{DPDK_DIR}/include)

set(components

flow_classify
pipeline
table
port
pdump
distributor
ip_frag
gro
gso
meter
lpm

acl

jobstats
metrics
bitratestats
latencystats
power
timer
efd

cfgfile
hash
member
vhost
kvargs
mbuf
net
ethdev
bbdev
cryptodev
security
eventdev
rawdev
mempool
mempool_ring
ring
pci
eal
cmdline
reorder
sched
kni
common_octeontx
bus_pci
bus_vdev
bus_dpaa
bus_fslmc
mempool_stack
mempool_dpaa
mempool_dpaa2
pmd_af_packet
pmd_ark
pmd_avf
pmd_avp
pmd_bnxt
pmd_bond
pmd_cxgbe
pmd_dpaa
pmd_dpaa2
pmd_e1000
pmd_ena
pmd_enic
pmd_fm10k
pmd_failsafe
pmd_i40e
pmd_ixgbe
pmd_kni
pmd_lio
pmd_nfp
pmd_null
pmd_qede
pmd_ring
pmd_softnic
pmd_sfc_efx
pmd_tap
pmd_thunderx_nicvf
pmd_vdev_netvsc
pmd_virtio
pmd_vhost
pmd_vmxnet3_uio
pmd_bbdev_null
pmd_null_crypto
pmd_crypto_scheduler
pmd_dpaa2_sec
pmd_dpaa_sec
pmd_skeleton_event
pmd_sw_event
pmd_octeontx_ssovf
pmd_dpaa_event
pmd_dpaa2_event
mempool_octeontx
pmd_octeontx
pmd_opdl_event
pmd_skeleton_rawdev
)

foreach(c ${components})
  find_library(DPDK_rte_${c}_LIBRARY rte_${c}
    HINTS $ENV{DPDK_DIR}/lib)
  # MESSAGE(STATUS "FOUND ${c}")
endforeach()

foreach(c ${components})
  list(APPEND check_LIBRARIES "${DPDK_rte_${c}_LIBRARY}")
  # MESSAGE(STATUS "${DPDK_rte_${c}_LIBRARY}")
endforeach()

mark_as_advanced(DPDK_INCLUDE_DIR ${check_LIBRARIES})

if (EXISTS ${WITH_DPDK_MLX5})
  find_library(DPDK_rte_pmd_mlx5_LIBRARY rte_pmd_mlx5)
  list(APPEND check_LIBRARIES ${DPDK_rte_pmd_mlx5_LIBRARY})
  mark_as_advanced(DPDK_rte_pmd_mlx5_LIBRARY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dpdk DEFAULT_MSG
  DPDK_INCLUDE_DIR
  check_LIBRARIES)

if(DPDK_FOUND)
  if(EXISTS ${WITH_DPDK_MLX5})
    list(APPEND check_LIBRARIES -libverbs)
  endif()
  set(DPDK_LIBRARIES
     -Wl,--whole-archive ${check_LIBRARIES} -Wl,--no-whole-archive
     -lnuma -lrt -lm -ldl
     )
endif(DPDK_FOUND)

MESSAGE(STATUS "GET DPDK include  ${DPDK_INCLUDE_DIR}")
MESSAGE(STATUS "GET DPDK libs     ${DPDK_LIBRARIES}")
