#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cal3d/cal3d.h>

#include "Log.h"
#include "Model.h"
#include "Profiler.h"
#include "ResourceManager.h"
#include "ScopedArray.h"
#include "ScopedPtr.h"
#include "Types.h"
#include "XMLParser.h"

namespace pyr {

    namespace {
        Logger& _logger = Logger::get("pyr.Model");
    }

    using std::ifstream;
    using std::string;
    using std::vector;

    /** A quick and dirty adapter for CalCoreModel.
     *
     * Since CalCoreModel must be destroy()ed before any CalModels using it,
     * I was forced to refcount them without using the existing ResourceManager
     * codebase.  Regardless, it's encapsulated, and not too convoluted.
     *
     * I'm not entirely sure how sound it is from a design standpoint, but
     * CoreModel also does the parsing of the config files, in addition to
     * a RAW texture loader. (which only needs to exist while I'm using models
     * included in the cal3d test demo)
     *
     */
    class CoreModel : public RefCounted {
    private:    
        typedef std::map<std::string,CoreModel*> InstanceMap;
        
        /// This static data kind of scares me.
        static InstanceMap _instances;

        CalCoreModel _coreModel;
        float _scale;
        float _matrix[4 * 4];

        // The names of each animation in order.  Use this to map names to ids.
        std::vector<std::string> _names;

        CoreModel(const std::string& id) {
            _coreModel.create(id);
            _scale = 1.0f;
            // Identity.
            std::fill(_matrix, _matrix + 16, 0.0f);
            for (int i = 0; i < 4; ++i) {
                _matrix[i * 4 + i] = 1;
            }
            loadConfigFile(id, _coreModel);
        }

    protected:
        ~CoreModel() {
            // gaaaaay.  But I can find no way to search a std::map for a certain value.
            for (InstanceMap::iterator i = _instances.begin(); i != _instances.end(); i++) {
                if (i->second == this) {
                    _instances.erase(i);
                    break;
                }
            }

            // deallocate textures
            for (int i = 0; i < _coreModel.getCoreMaterialCount(); i++) {
                CalCoreMaterial& material = *_coreModel.getCoreMaterial(i);

                for (int j = 0; j < material.getMapCount(); j++) {
                    CalTexture* tex = static_cast<CalTexture*>(material.getMapUserData(j));
                    glDeleteTextures(1, &tex->tex);
                    delete tex;
                }
            }

            // deallocate the model
            _coreModel.destroy();
        }

    private:
        // 'x' -> 1, 'y' -> 2, 'z' -> 3
        static int getAxisIndex(char c) {
            switch (tolower(c)) {
                case 'x': return 1;
                case 'y': return 2;
                case 'z': return 3;
                default:  return 0;
            }
        }

        static bool setRow(float row[3], const string& a) {
            int column = 0;
            if (a.length() == 1) {
                column = getAxisIndex(a[0]);
            } else if (a.length() == 2) {
                if (a[0] == '+') column = +getAxisIndex(a[1]);
                if (a[0] == '-') column = -getAxisIndex(a[1]);
            }

            if (column > 0) { std::fill(row, row + 3, 0.0f); row[ column - 1] =  1; }
            if (column < 0) { std::fill(row, row + 3, 0.0f); row[-column - 1] = -1; }
            return column != 0;
        }

        void loadConfigFile(const string& filename, CalCoreModel& model) {
            ScopedPtr<XMLNode> modelRoot;
            try {
                modelRoot = parseXMLFile(filename);
                if (!modelRoot) {
                    throw LoadModelError("Couldn't parse " + filename);
                }
            }
            catch (const XMLParseError& error) {
                throw LoadModelError("Error parsing " + filename + ": " + error.what());
            }

            if (modelRoot->getName() != "model") {
                throw LoadModelError("Model configuration file is invalid (root name should be 'model')");
            }

            _scale = float(atof(modelRoot->getAttr("scale", "1").c_str()));

            if (modelRoot->hasAttr("orientation")) {
                std::istringstream is(modelRoot->getAttr("orientation"));
                string axis[3];
                if (is >> axis[0] >> axis[1] >> axis[2]) {
                    // Get the bases for rotating the model into "animation space".
                    float matrix[3 * 3];
                    bool valid = true;
                    for (int i = 0; i < 3; ++i) {
                        valid = valid && setRow(matrix + i * 3, axis[i]);
                    }
                    if (valid) {
                        // Transpose and plug the rotated bases in.
                        for (int i = 0; i < 3; ++i) {
                            for (int j = 0; j < 3; ++j) {
                                _matrix[i * 4 + j] = matrix[j * 3 + i];
                            }
                        }
                    }
                }
            }

            string skeleton;
            vector<string> animations;
            vector<string> meshes;
            vector<string> materials;

            _names.clear();

            for (size_t i = 0; i < modelRoot->getChildCount(); ++i) {
                XMLNode* child = modelRoot->getChild(i);
                if (child->getName() == "skeleton") {
                    skeleton = child->getAttr("file");
                } else if (child->getName() == "animation") {
                    _names.push_back(child->getAttr("name"));
                    animations.push_back(child->getAttr("file"));
                } else if (child->getName() == "mesh") {
                    meshes.push_back(child->getAttr("file"));
                } else if (child->getName() == "material") {
                    materials.push_back(child->getAttr("file"));
                } else if (!child->getName().empty()) {
                    PYR_LOG(_logger, WARN, "Unknown child node of model: " << child->getName());
                }
            }

            doLoad(filename, skeleton, animations, meshes, materials, model);
        }

        void doLoad(const string& filename,
                    const string& skeleton,
                    const vector<string>& animations,
                    const vector<string>& meshes,
                    const vector<string>& materials,
                    CalCoreModel& model
        ) {
            string path = getPath(filename);
            PYR_LOG(_logger, INFO, "Model Load Path " << path);

            // Actually load things, now that the config file has been consumed.
            struct CalException {};
            try {
                int result;

                result=model.loadCoreSkeleton(path + skeleton);
                if (!result)        throw CalException();

                for (u32 i = 0; i < animations.size(); i++) {
                    result=model.loadCoreAnimation(path + animations[i]);
                    PYR_LOG(_logger, INFO, "Loading " << animations[i]);
                    if (result==-1) throw CalException();
                }

                for (u32 i = 0; i < meshes.size(); i++) {
                    result=model.loadCoreMesh(path + meshes[i]);
                    if (result==-1) throw CalException();
                }

                for (u32 i = 0; i < materials.size(); i++) {
                    result=model.loadCoreMaterial(path + materials[i]);
                    if (result==-1) throw CalException();
                }
            }
            catch (const CalException&) {
                std::ostringstream ss;
                ss << "CoreModel::loadConfigFile " << filename << ": " <<  CalError::getLastErrorText() << std::endl;
                throw LoadModelError(ss.str().c_str());
            }

            // Load textures
            for (int i = 0; i < model.getCoreMaterialCount(); i++) {
                CalCoreMaterial& material=*model.getCoreMaterial(i);

                for (int j = 0; j < material.getMapCount(); j++) {
                    string s=material.getMapFilename(j);
                    CalTexture* hTex=loadTexture(path + s); // for now, loading RAW textures.  TODO: replace with corona
                    material.setMapUserData(j,hTex);
                }
            }

            // Assign material threads to materials.
            for (int i = 0; i < model.getCoreMaterialCount(); i++) {
                model.createCoreMaterialThread(i);
                model.setCoreMaterialId(i,0,i);
            }
        }

        static u32 read32_le(u8* buf) {
            return
                (buf[3] << 24) +
                (buf[2] << 16) +
                (buf[1] << 8) +
                buf[0];
        }

        //! Temporary RAW texture loader.
        static CalTexture* loadTexture(const string& fname) {
            ifstream file(fname.c_str(), std::ios::in | std::ios::binary);
            if (!file) {
                return 0;
            }

            u8 buf[12];
            file.read((char*)buf, 12);

            int w = (int)read32_le(buf);
            int h = (int)read32_le(buf + 4);
            int d = (int)read32_le(buf + 8);

            ScopedArray<u8> pixels(new u8[w * h * d]);
            u8* p=pixels.get() + (h - 1) * w * d;

            // RAWs are stored bottom-to-top.  Flip as we read
            for (int i = 0; i < h; i++) {
                file.read((char*)p, w * d);
                p -= w * d;
            }

            file.close();

            int pixelformat = (d == 3) ? GL_RGB : GL_RGBA;

            CalTexture* tex = new CalTexture;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glGenTextures(1, &tex->tex);
            glBindTexture(GL_TEXTURE_2D, tex->tex);
            glTexImage2D(GL_TEXTURE_2D, 0, pixelformat, w, h, 0, pixelformat, GL_UNSIGNED_BYTE, pixels.get());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            return tex;
        }

    public:
        //! "constructor"
        static CoreModel* create(const std::string& id) {
            CoreModel* m=_instances[id];

            if (!m) {
                m=new CoreModel(id);
                _instances[id]=m;
            }

            return m;
        }

        CalCoreModel* get() {
            return &_coreModel;
        }

        float getScale() const {
            return _scale;
        }

        const float* getMatrix() const {
            return _matrix;
        }
        
        int getAnimationID(const std::string& name) {
            for (size_t i = 0; i < _names.size(); ++i) {
                if (_names[i] == name) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }
    };

    CoreModel::InstanceMap CoreModel::_instances;

    Model::Model(const string& fname) {
        _coreModel=CoreModel::create(fname);

        _model.create(_coreModel->get());

        for (int i = 0; i < _coreModel->get()->getCoreMeshCount(); i++)
            _model.attachMesh(i);

        _model.setMaterialSet(0);
        _model.setLodLevel(5);
        _scale = _coreModel->getScale();
    }

    Model::~Model() {
        _model.destroy();
    }

    CalCoreModel& Model::getCoreModel() const {
        return *_coreModel->get();
    }

    CalModel& Model::getModel() {
        return _model;
    }

    const CalModel& Model::getModel() const {
        return _model;
    }

    const float* Model::getMatrix() const {
        return _coreModel->getMatrix();
    }

    void Model::update(float timedelta) {
        PYR_PROFILE_BLOCK("Model::update");
        _model.update(timedelta);
    }

    void Model::blendCycle(const std::string& animation, float weight, float delay) {
        int id = _coreModel->getAnimationID(animation);
        if (id != -1) {
            if (_lastCycle != -1) {
                _model.getMixer()->clearCycle(_lastCycle, 0.1f);
            }
            _lastCycle = id;
            _model.getMixer()->blendCycle(id, weight, delay);
        } else {
            PYR_LOG(_logger, WARN, "animation '" << animation << "' does not map to an id");
        }
    }

    void Model::executeAction(const std::string& animation, float weight, float delayIn, float delayOut) {
        int id = _coreModel->getAnimationID(animation);
        if (id != -1) {
            _model.getMixer()->executeAction(id, delayIn, delayOut /*, weight*/);
        } else {
            PYR_LOG(_logger, WARN, "animation '" << animation << "' does not map to an id");
        }
    }

}
