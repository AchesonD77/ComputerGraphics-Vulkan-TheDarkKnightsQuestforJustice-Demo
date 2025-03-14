
struct PipelineInstances;

struct Instance {
	std::string *id;
	int Mid;
	int NTx;
	int Iid;
	int *Tid;
	DescriptorSet **DS;
	std::vector<DescriptorSetLayout *> *D;
	int NDs;
	
	glm::mat4 Wm;
	PipelineInstances *PI;
} ;

struct PipelineRef {
	std::string *id;
	Pipeline *P;
	VertexDescriptor *VD;

	void init(const char *_id, Pipeline * _P, VertexDescriptor * _VD) {
		id = new std::string(_id);
		P = _P;
		VD = _VD;
	}} ;

struct VertexDescriptorRef {
	std::string *id;
	VertexDescriptor *VD;
	
	void init(const char *_id, VertexDescriptor * _VD) {
		id = new std::string(_id);
		VD = _VD;
	}
} ;

struct PipelineInstances {
	Instance *I;
	int InstanceCount;
	PipelineRef *P;
} ;


class Scene {
	public:
	
	BaseProject *BP;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	// Models
	int ModelCount = 0;
	Model **M;
	std::unordered_map<std::string, int> MeshIds;

	// Textures
	int TextureCount = 0;
	Texture **T;
	std::unordered_map<std::string, int> TextureIds;
	
	// Descriptor sets and instances
	int InstanceCount = 0;

	Instance **I;
	VertexDescriptorRef *VRef;
	std::unordered_map<std::string, int> InstanceIds;

	// Pipelines, DSL and Vertex Formats
	std::unordered_map<std::string, PipelineRef *> PipelineIds;
	int PipelineInstanceCount = 0;
	PipelineInstances *PI;
	std::unordered_map<std::string, VertexDescriptor *> VDIds;


	int init(BaseProject *_BP,  std::vector<VertexDescriptorRef>  &VDRs,  
			  std::vector<PipelineRef> &PRs, std::string file) {
		BP = _BP;
		
		for(int i = 0; i < VDRs.size(); i++) {
			VDIds[*VDRs[i].id] = VDRs[i].VD;
		}
		for(int i = 0; i < PRs.size(); i++) {
			PipelineIds[*PRs[i].id] = &PRs[i];
		}

		// Models, textures and Descriptors (values assigned to the uniforms)
		nlohmann::json js;
		std::ifstream ifs("models/scene.json");
		if (!ifs.is_open()) {
		  std::cout << "Error! Scene file not found!";
		  exit(-1);
		}
//		try {
			std::cout << "Parsing JSON\n";
			ifs >> js;
			ifs.close();
			std::cout << "\nScene contains " << js.size() << " definitions sections\n\n";
			
			// MODELS
			nlohmann::json ms = js["models"];
			ModelCount = ms.size();
			std::cout << "Models count: " << ModelCount << "\n";

			M = (Model **)calloc(ModelCount, sizeof(Model *));
			for(int k = 0; k < ModelCount; k++) {
				MeshIds[ms[k]["id"]] = k;
				std::string MT = ms[k]["format"].template get<std::string>();
				std::string VDN = ms[k]["VD"].template get<std::string>();

				M[k] = new Model();
				M[k]->init(BP, VDIds[VDN], ms[k]["model"], (MT[0] == 'O') ? OBJ : ((MT[0] == 'G') ? GLTF : MGCG));
			}
			
			// TEXTURES
			nlohmann::json ts = js["textures"];
			TextureCount = ts.size();
			std::cout << "Textures count: " << TextureCount << "\n";

			T = (Texture **)calloc(ModelCount, sizeof(Texture *));
			for(int k = 0; k < TextureCount; k++) {
				TextureIds[ts[k]["id"]] = k;
				std::string TT = ts[k]["format"].template get<std::string>();

				T[k] = new Texture();
				if(TT[0] == 'C') {
					T[k]->init(BP, ts[k]["texture"]);
				} else if(TT[0] == 'D') {
					T[k]->init(BP, ts[k]["texture"], VK_FORMAT_R8G8B8A8_UNORM);
				} else {
					std::cout << "FORMAT UNKNOWN: " << TT << "\n";
				}
std::cout << ts[k]["id"] << "(" << k << ") " << TT << "\n";
			}

			// INSTANCES TextureCount
			nlohmann::json pis = js["instances"];
			PipelineInstanceCount = pis.size();
std::cout << "Pipeline Instances count: " << PipelineInstanceCount << "\n";
			PI = (PipelineInstances *)calloc(PipelineInstanceCount, sizeof(PipelineInstances));
			InstanceCount = 0;

			for(int k = 0; k < PipelineInstanceCount; k++) {
				std::string Pid = pis[k]["pipeline"].template get<std::string>();
				
				PI[k].P = PipelineIds[Pid];
				nlohmann::json is = pis[k]["elements"];
				PI[k].InstanceCount = is.size();
std::cout << "Pipeline: " << Pid << "(" << k << "), Instances count: " << PI[k].InstanceCount << "\n";
				PI[k].I = (Instance *)calloc(PI[k].InstanceCount, sizeof(Instance));
				
				for(int j = 0; j < PI[k].InstanceCount; j++) {
				
std::cout << k << "." << j << "\t" << is[j]["id"] << ", " << is[j]["model"] << "(" << MeshIds[is[j]["model"]] << "), {";
					PI[k].I[j].id  = new std::string(is[j]["id"]);
					PI[k].I[j].Mid = MeshIds[is[j]["model"]];
					int NTextures = is[j]["texture"].size();
					PI[k].I[j].NTx = NTextures;
					PI[k].I[j].Tid = (int *)calloc(NTextures, sizeof(int));
std::cout << "#" << NTextures;
					for(int h = 0; h < NTextures; h++) {
						PI[k].I[j].Tid[h] = TextureIds[is[j]["texture"][h]];
std::cout << " " << is[j]["texture"][h] << "(" << PI[k].I[j].Tid[h] << ")";
					}
std::cout << "}\n";
					nlohmann::json TMjson = is[j]["transform"];
					float TMj[16];
					for(int h = 0; h < 16; h++) {TMj[h] = TMjson[h];}
					PI[k].I[j].Wm = glm::mat4(TMj[0],TMj[4],TMj[8],TMj[12],TMj[1],TMj[5],TMj[9],TMj[13],TMj[2],TMj[6],TMj[10],TMj[14],TMj[3],TMj[7],TMj[11],TMj[15]);
					
					PI[k].I[j].PI = &PI[k];
					PI[k].I[j].D = &PI[k].P->P->D;
					PI[k].I[j].NDs = PI[k].I[j].D->size();
					BP->DPSZs.setsInPool += PI[k].I[j].NDs;

					for(int h = 0; h < PI[k].I[j].NDs; h++) {
						DescriptorSetLayout *DSL = (*PI[k].I[j].D)[h];
						int DSLsize = DSL->Bindings.size();

						for (int l = 0; l < DSLsize; l++) {
							if(DSL->Bindings[l].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
								BP->DPSZs.uniformBlocksInPool += 1;
							} else {
								BP->DPSZs.texturesInPool += 1;
							}
						}
					}
					InstanceCount++;
				}
			}			

std::cout << "Creating instances\n";
			I =  (Instance **)calloc(InstanceCount, sizeof(Instance *));

			int i = 0;
			for(int k = 0; k < PipelineInstanceCount; k++) {
				for(int j = 0; j < PI[k].InstanceCount; j++) {
					I[i] = &PI[k].I[j];
					InstanceIds[*I[i]->id] = i;
					I[i]->Iid = i;
					
					i++;
				}
			}
std::cout << i << " instances created\n";


/*		} catch (const nlohmann::json::exception& e) {
			std::cout << "\n\n\nException while parsing JSON file: " << file << "\n";
			std::cout << e.what() << '\n' << '\n';
			std::cout << std::flush;
			return 1;
		}
*/
std::cout << "Leaving scene loading and creation\n";		
		return 0;
	}


	void pipelinesAndDescriptorSetsInit() {
std::cout << "Scene DS init\n";
		for(int i = 0; i < InstanceCount; i++) {
std::cout << "I: " << i << ", NTx: " << I[i]->NTx << ", NDs: " << I[i]->NDs << "\n";
			Texture **Tids = (Texture **)calloc(I[i]->NTx, sizeof(Texture *));
			for(int j = 0; j < I[i]->NTx; j++) {
				Tids[j] = T[I[i]->Tid[j]];
			}

			I[i]->DS = (DescriptorSet **)calloc(I[i]->NDs, sizeof(DescriptorSet *));
			for(int j = 0; j < I[i]->NDs; j++) {
				I[i]->DS[j] = new DescriptorSet();
				I[i]->DS[j]->init(BP, (*I[i]->D)[j], Tids);
			}
			free(Tids);
		}
std::cout << "Scene DS init Done\n";
	}
	
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup datasets
		for(int i = 0; i < InstanceCount; i++) {
			for(int j = 0; j < I[i]->NDs; j++) {
				I[i]->DS[j]->cleanup();
				delete I[i]->DS[j];
			}
			free(I[i]->DS);
		}
	}

	void localCleanup() {
		// Cleanup textures
		for(int i = 0; i < TextureCount; i++) {
			T[i]->cleanup();
			delete T[i];
		}
		free(T);
		
		// Cleanup models
		for(int i = 0; i < ModelCount; i++) {
			M[i]->cleanup();
			delete M[i];
		}
		free(M);
		
		for(int i = 0; i < InstanceCount; i++) {
			delete I[i]->id;
			free(I[i]->Tid);
		}
		free(I);
		
		// To add: delete the also the datastructure relative to the pipeline
		for(int i = 0; i < PipelineInstanceCount; i++) {
			free(PI[i].I);
		}
		free(PI);
	}
	
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		for(int k = 0; k < PipelineInstanceCount; k++) {
			for(int i = 0; i < PI[k].InstanceCount; i++) {
				Pipeline *P = PI[k].I[i].PI->P->P;
				P->bind(commandBuffer);

	//std::cout << "Drawing Instance " << i << "\n";
				M[PI[k].I[i].Mid]->bind(commandBuffer);
	//std::cout << "Binding DS: " << DS[i] << "\n";
				for(int j = 0; j < PI[k].I[i].NDs; j++) {
					PI[k].I[i].DS[j]->bind(commandBuffer, *P, j, currentImage);
				}

	//std::cout << "Draw Call\n";						
				vkCmdDrawIndexed(commandBuffer,
						static_cast<uint32_t>(M[PI[k].I[i].Mid]->indices.size()), 1, 0, 0, 0);
			}
		}
	}
};
    
