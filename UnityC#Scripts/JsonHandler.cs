using System.Collections;
using System.Collections.Generic;
using System;
using UnityEngine;

/******************************************************************
 * Json Handler:
 * contains all functions for reading and writing game data
 *    to and from json
 * 
 * ***************************************************************/
public class JsonHandler
{
    //creates a new file and saves it to the json file
    public void newFile(string fileName)
    {
        GameInfo gameInfo = getGameInfo();

        FileInfo fileInfo = new FileInfo();
        fileInfo.name = fileName;
        fileInfo.levels = new List<LevelInfo>();

        //add all the levels, unlock the first level
        fileInfo.levels.Add(new LevelInfo("Level1"));
        fileInfo.levels[0].unlocked = true;
        gameInfo.files.Add(fileInfo);
        saveGameInfo(gameInfo);
    }

    //gets GameInfo object
    public GameInfo getGameInfo()
    {
        GameInfo gameInfo = new GameInfo();
        try
        {
            string readJson = System.IO.File.ReadAllText(Application.persistentDataPath + "/SaveData.json");
            gameInfo = JsonUtility.FromJson<GameInfo>(readJson);
        } catch (System.IO.FileNotFoundException e)
        {
            Debug.Log("Json Doesn't exist: " + e);
            saveGameInfo(gameInfo);
        }
        return gameInfo;
    }

    //saves GameInfo object
    public void saveGameInfo(GameInfo gameInfo)
    {
        string writeJson = JsonUtility.ToJson(gameInfo, true);
        System.IO.File.WriteAllText(Application.persistentDataPath + "/SaveData.json", writeJson);
    }

    //returns the index of the file with the given name, returns -1 if it is not found
    public int getFileIndex(string name)
    {
        GameInfo gameInfo = getGameInfo();

        //FileInfo searchFile;
        for(int i = 0; i < gameInfo.files.Count; i++)
        {
            if(gameInfo.files[i].name == name)
            {
                return i;
            }
        }
        //not found
        Debug.Log("File not found");
        return -1;
    }

    //saves file to json
    public void saveFile(FileInfo fileToSave)
    {
        GameInfo gameInfo = getGameInfo();

        for (int i = 0; i < gameInfo.files.Count; i++)
        {
            if (gameInfo.files[i].name == fileToSave.name)
            {
                //file exsists
                gameInfo.files[i] = fileToSave;
                saveGameInfo(gameInfo);
                return;
            }
        }
    }


    //returns level index of given name and fileIndex, returns -1 if not found
    public int getLevelIndex(int fileIndex, string levelName)
    {
        FileInfo file = getGameInfo().files[fileIndex];
        if(file == null)
        {
            Debug.Log("here");
        }
        for(int i = 0; i < file.levels.Count; i++)
        {

            if (file.levels[i].name == levelName)
            {
                return i;
            }
        }
        return -1;
    }

    //saves the scroll info to json
    public void saveScroll(string fileName, string levelName, ScrollInfo scroll)
    {
        GameInfo gameInfo = getGameInfo();
        int fileIndex = getFileIndex(fileName);
        int levelIndex = getLevelIndex(fileIndex, levelName);
        if(levelIndex == -1)
        {
            Debug.Log("level is null");
        }
        gameInfo.files[fileIndex].levels[levelIndex].scrolls.Add(scroll);
        saveGameInfo(gameInfo);
    }

    //returns all ScrollInfo objects of a given level
    public List<ScrollInfo> getScrolls(int fileIndex, int levelIndex)
    {
        GameInfo gameInfo = getGameInfo();
        FileInfo fileInfo = gameInfo.files[fileIndex];
        LevelInfo levelInfo = fileInfo.levels[levelIndex];
        return levelInfo.scrolls;
    }

    //deletes given level's scrolls
    public void clearScrolls(int fileIndex, int levelIndex)
    {
        GameInfo gameInfo = getGameInfo();
        FileInfo fileInfo = gameInfo.files[fileIndex];
        LevelInfo levelInfo = fileInfo.levels[levelIndex];
        levelInfo.scrolls = new List<ScrollInfo>();
        saveGameInfo(gameInfo);
    }

    //deletes file
    public void deleteFile(string fileName)
    {
        GameInfo gameInfo = getGameInfo();

        FileInfo fileToDelete = null;
        for (int i = 0; i < gameInfo.files.Count; i++)
        {
            if (gameInfo.files[i].name == fileName)
            {
                //file exsists
                fileToDelete = gameInfo.files[i];
            }
        }
        if(fileToDelete != null)
        {
            gameInfo.files.Remove(fileToDelete);
        }
        return;
    }
}

[Serializable]
public class MathProblem
{
    public int level;
    public int first;
    public int second;
    public int third;
    public int split;
    public int equals;
    public int input;
}
[Serializable]
public class ScrollInfo
{
    public bool isComplete;
    public bool isPassed;
    public MathProblem problem;
}

[Serializable]
public class LevelInfo
{
    public string name;
    public bool unlocked;
    public bool passed;
    public int topScore;
    public List<ScrollInfo> scrolls;

    public LevelInfo(string levelName)
    {
        name = levelName;
        unlocked = false;
        passed = false;
        topScore = 0;
        scrolls = new List<ScrollInfo>();
    }
}
[Serializable]
public class FileInfo
{
    public string name;
    public List<LevelInfo> levels;
    public FileInfo()
    {
        levels = new List<LevelInfo>();
    }
}
[Serializable]
public class GameInfo
{
    public List<FileInfo> files;

    public GameInfo()
    {
        files = new List<FileInfo>();
    }
}





