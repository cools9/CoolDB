package main

import (
	"cooldb/cooldb"
	"net/http"

	"github.com/gin-gonic/gin"
)

func main() {
	router := gin.Default()

	// Endpoint to set a key-value pair
	router.POST("/set", func(c *gin.Context) {
		var request struct {
			Key   string `json:"key"`
			Value string `json:"value"`
		}

		if err := c.BindJSON(&request); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid request"})
			return
		}

		cooldb.Set(request.Key, request.Value)
		c.JSON(http.StatusOK, gin.H{"message": "Key-value pair set successfully"})
	})

	// Endpoint to get a value by key
	router.GET("/get/:key", func(c *gin.Context) {
		key := c.Param("key")
		value, found := cooldb.Get(key)
		if found {
			c.JSON(http.StatusOK, gin.H{"key": key, "value": value})
		} else {
			c.JSON(http.StatusNotFound, gin.H{"error": "Key not found"})
		}
	})

	// Endpoint to list all keys
	router.GET("/list", func(c *gin.Context) {
		keys := cooldb.List()
		c.JSON(http.StatusOK, gin.H{"keys": keys})
	})

	// Endpoint to get the status of the database
	router.GET("/status", func(c *gin.Context) {
		status := cooldb.Status()
		c.JSON(http.StatusOK, gin.H{"status": status})
	})

	// Start the server on localhost:8080
	router.Run("localhost:8080")
}
