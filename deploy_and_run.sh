#!/bin/bash

set -e

# Configuration
SSH_HOST="giovanni.feltrin@hpc2.unitn.it" # run `ssh-copy-id giovanni.feltrin@hpc2.unitn.it` first to save password
LOCAL_DIR="./current"
REMOTE_DIR="~/hpc_jobs"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
REMOTE_JOB_DIR="${REMOTE_DIR}/job_${TIMESTAMP}"

echo "========================================="
echo "HPC Job Deployment Script"
echo "========================================="
echo "Target: ${SSH_HOST}"
echo "Local directory: ${LOCAL_DIR}"
echo "Remote directory: ${REMOTE_JOB_DIR}"
echo "========================================="

# Copy files to remote server
echo "Copying files to HPC server..."
rsync -avz --progress ${LOCAL_DIR}/ ${SSH_HOST}:${REMOTE_JOB_DIR}/

if [ $? -ne 0 ]; then
    echo "Error: Failed to copy files"
    exit 1
fi

echo "Files copied successfully!"

# Make the PBS script executable and submit job
echo "Setting permissions and submitting PBS job..."
JOB_ID=$(ssh ${SSH_HOST} "cd ${REMOTE_JOB_DIR} && qsub run_mpi.sh" | cut -d'.' -f1)

if [ -z "$JOB_ID" ]; then
    echo "Error: Failed to submit PBS job"
    exit 1
fi

echo "Job submitted successfully! Job ID: ${JOB_ID}"

echo ""
echo "========================================="
echo "Job Information"
echo "========================================="
echo "Job ID: ${JOB_ID}"
echo "Remote directory: ${REMOTE_JOB_DIR}"
echo ""
echo "Useful commands:"
echo "  Check status:  ssh ${SSH_HOST} 'qstat -u giovanni.feltrin'"
echo "  Check job:     ssh ${SSH_HOST} 'qstat ${JOB_ID}'"
echo "  View output:   ssh ${SSH_HOST} 'cat ${REMOTE_JOB_DIR}/run_mpi.sh.o*'"
echo "  View errors:   ssh ${SSH_HOST} 'cat ${REMOTE_JOB_DIR}/run_mpi.sh.e*'"
echo "  Cancel job:    ssh ${SSH_HOST} 'qdel ${JOB_ID}'"
echo "  List files:    ssh ${SSH_HOST} 'ls -lh ${REMOTE_JOB_DIR}'"
echo "  SSH to dir:    ssh ${SSH_HOST} -t 'cd ${REMOTE_JOB_DIR} && bash'"
echo "========================================="

echo ""
echo "Waiting for job to complete..."

# Wait for job to complete
while true; do
    JOB_STATUS=$(ssh ${SSH_HOST} "qstat ${JOB_ID} 2>/dev/null | tail -n 1 | awk '{print \$5}'")

    if [ -z "$JOB_STATUS" ]; then
        echo "Job completed!"
        break
    fi

    echo "Job status: ${JOB_STATUS} - waiting..."
    sleep 2
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
