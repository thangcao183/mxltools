#!/bin/bash

echo "🔍 FOCUSED SEGFAULT TESTING"
echo "=========================="

cd /home/wolf/CODE/C/mxltools/build

echo ""
echo "1. Testing Property Addition WITHOUT Save:"
echo "   - Load character"
echo "   - Move item to shared stash"
echo "   - Add property"
echo "   - DON'T save - just exit"
echo ""

echo "2. Testing Save WITHOUT Property Addition:"
echo "   - Load character"
echo "   - Make NO changes"
echo "   - Save directly"
echo ""

echo "3. Testing Full Workflow with Valgrind:"
echo "   - Load character"
echo "   - Add property to shared stash item"
echo "   - Save (watch for segfault)"
echo ""

echo "Running with NVIDIA suppressions..."
valgrind --tool=memcheck \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --suppressions=../nvidia.supp \
         ./MedianXLOfflineTools

echo ""
echo "💡 ANALYSIS FOCUS:"
echo "   - Does segfault happen during property addition or during save?"
echo "   - Is it related to SimplePropertyInserter or save process?"
echo "   - What's the exact stack trace at crash?"