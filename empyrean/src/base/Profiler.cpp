#include <fstream>
#include <prinrval.h>
#include "Profiler.h"
#include "NSPRUtility.h"

namespace pyr {

    static const size_t REMEMBERED_FRAMES = 60;

    
    // class ProfileBlock

    ProfileBlock::ProfileBlock(const std::string& name) {
        _name = name;
        lastFrames.resize(REMEMBERED_FRAMES);
    }

    void ProfileBlock::nextFrame() {
        for (int i = REMEMBERED_FRAMES - 1; i > 0; --i) {
            lastFrames[i] = lastFrames[i - 1];
        }
        lastFrames[0] = ProfileData();
    }

    float ProfileBlock::getAverageTime() const {
        float total = 0;
        for (size_t i = 0; i < REMEMBERED_FRAMES; ++i) {
            total += lastFrames[i].time();
        }
        return total / REMEMBERED_FRAMES;
    }

    float ProfileBlock::getAverageTotalTime() const {
        float total = 0;
        for (size_t i = 0; i < REMEMBERED_FRAMES; ++i) {
            total += lastFrames[i].timePlusChildren;
        }
        return total / REMEMBERED_FRAMES;
    }


    // class Profiler

    PYR_DEFINE_SINGLETON(Profiler)

    void Profiler::beginBlock(const std::string& name) {
        ProfileBlockPtr block = _blocks[name];
        if (!block) {
            block = new ProfileBlock(name);
            _blocks[name] = block;
            block->current = false;
        }

        PYR_ASSERT(!block->current, "Block can't be profiled recursively.");

        block->current   = true;
        block->entryTime = getNow();
        pushBlock(block);
    }

    void Profiler::endBlock() {
        ProfileBlockPtr block = popBlock();
        PYR_ASSERT(block->current, "Can't end block that isn't started.");

        float dt = getNow() - block->entryTime;
        if (dt > 0) { // ignore timer wraparound
            
            // Only record total time for top-level profiling blocks.
            if (_callStack.empty()) {
                _totalTime += dt;
            }

            block->update(dt);

            // If call stack isn't empty, update parent with time in children.
            if (!_callStack.empty()) {
                _callStack.top()->block->updateParent(dt);
            }
        }
        block->current = false;
    }

    void Profiler::nextFrame() {
        // For now, require that nextFrame() has to be called outside of profiling blocks.
        PYR_ASSERT(_callStack.empty(), "nextFrame() must be called from outside of profiling blocks.");
        _lastCallTree = _callTree;
        _callTree.clear();

        for (ProfileBlockMap::iterator i = _blocks.begin(); i != _blocks.end(); ++i) {
            i->second->nextFrame();
        }
    }

    void Profiler::pushBlock(ProfileBlockPtr block) {
        CallNode* cn = new CallNode;
        cn->block = block;
        
        if (_callStack.empty()) {
            _callTree.push_back(cn);
        } else {
            _callStack.top()->children.push_back(cn);
        }

        _callStack.push(cn);
    }

    ProfileBlockPtr Profiler::popBlock() {
        PYR_ASSERT(!_callStack.empty(), "endBlock() called when call stack is empty.");

        ProfileBlockPtr top = _callStack.top()->block;
        _callStack.pop();
        return top;
    }
};