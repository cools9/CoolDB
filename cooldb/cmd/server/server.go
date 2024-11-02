package main

import (
	"cooldb/internal/cooldb"
	"net/http"

	"github.com/gin-gonic/gin"
)

func main() {
	router := gin.Default()
	db := cooldb.New()

	router.POST("/set", func(c *gin.Context) {
		var request struct {
			Key   string `json:"key"`
			Value string `json:"value"`
		}
		if err := c.BindJSON(&request); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid request"})
			return
		}

		db.Set(request.Key, request.Value)
		c.JSON(http.StatusOK, gin.H{"message": "Key-value pair set successfully"})
	})

	router.GET("/get/:key", func(c *gin.Context) {
		key := c.Param("key")
		value, found := db.Get(key)
		if found {
			c.JSON(http.StatusOK, gin.H{"key": key, "value": value})
		} else {
			c.JSON(http.StatusNotFound, gin.H{"error": "Key not found"})
		}
	})

	router.GET("/list", func(c *gin.Context) {
		keys := db.List()
		c.JSON(http.StatusOK, gin.H{"keys": keys})
	})

	router.GET("/status", func(c *gin.Context) {
		status := db.Status()
		c.JSON(http.StatusOK, gin.H{"status": status})
	})

	router.Run("localhost:8080")
}
