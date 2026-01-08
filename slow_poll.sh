#!/bin/bash

set -e

# Check arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <ssh_host> <remote_job_dir>"
    echo "Example: $0 giovanni.feltrin@hpc2.unitn.it ~/hpc_jobs/job_20240115_123456"
    exit 1
fi

SSH_HOST="$1"
REMOTE_JOB_DIR="$2"

echo "========================================="
echo "HPC Job Monitor Script"
echo "========================================="
echo "SSH Host: ${SSH_HOST}"
echo "Remote directory: ${REMOTE_JOB_DIR}"
echo "========================================="

# Extract job ID from the remote directory
echo "Finding job ID..."
JOB_ID=$(ssh ${SSH_HOST} "cd ${REMOTE_JOB_DIR} && ls run_mpi.sh.o* 2>/dev/null | head -n1 | sed 's/.*\.o//'" 2>/dev/null || echo "")

if [ -z "$JOB_ID" ]; then
    echo "Warning: Could not find job ID from output files. Trying qstat..."
    JOB_ID=$(ssh ${SSH_HOST} "qstat -u \$(whoami) | tail -n 1 | awk '{print \$1}' | cut -d'.' -f1" 2>/dev/null || echo "")
fi

if [ -z "$JOB_ID" ]; then
    echo "Error: Could not determine job ID"
    exit 1
fi

echo "Monitoring Job ID: ${JOB_ID}"
echo ""

# Wait for job to complete
echo "Polling job status every 30 seconds..."
while true; do
    JOB_STATUS=$(ssh ${SSH_HOST} "qstat ${JOB_ID} 2>/dev/null | tail -n 1 | awk '{print \$5}'")

    if [ -z "$JOB_STATUS" ]; then
        echo "Job completed!"
        break
    fi

    echo "[$(date +%H:%M:%S)] Job status: ${JOB_STATUS}"
    sleep 30
done

# Display results
echo ""
echo "========================================="
echo "Job Errors (if any):"
echo "========================================="
ssh ${SSH_HOST} "cat ${REMOTE_JOB_DIR}/run_mpi.sh.e* 2>/dev/null"

echo ""
echo "========================================="
echo "Job Output:"
echo "========================================="
ssh ${SSH_HOST} "cat ${REMOTE_JOB_DIR}/run_mpi.sh.o* 2>/dev/null"

echo ""
echo "========================================="
echo "Monitoring Complete"
echo "========================================="
