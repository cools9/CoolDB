export class CoolDBClient {
	#baseUrl;
	
	constructor(baseUrl = "http://localhost:8080") {
  	this.#baseUrl = baseUrl;
  }
  
  async #fetchJson(endpoint, options = {}) {
    const defaultHeaders = {
      "Accept": "application/json",
    	"Content-Type": "application/json"
    };
    
    try {
    	const response = await fetch(`${this.#baseUrl}${endpoint}`, {
      	...options,
        headers: {
        	...defaultHeaders,
          ...options.headers
        }
      });
      
      const json = await response.json();
      
      if (response.ok) {
      	return json;
      }
      
      throw new Error(json.error ?? `HTTP error - status: ${response.status}`);
    } catch (error) {
    	if (error.message) throw new Error(`API request failed: ${error.message}`);
      
      throw new Error('Network request failed');
    }
  }
  
  async set(key, value) {  	
  	const json = await this.#fetchJson("/set", {
    	method: "POST",
      body: JSON.stringify({ key, value })
    });
    return json.value;
  }
  
  async get(key) {  	
  	if (!key) throw new Error("Key is required");
    const json = await this.#fetchJson(`/get/${encodeURIComponent(key)}`);
    return json.value;
  }
  
  async list() {
  	const json = await this.#fetchJson("/list");
    return json.keys;
  }
  
  async status() {  	
  	const json = await this.#fetchJson("/status");
    return json.status;
  }
}
