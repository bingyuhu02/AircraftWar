## Aircraft War

A Kinect motion control game developed by **Mystic**.

```
Team members: Yuechuan Xue, Sen Shen and Feifei Wang
Hardware: Microsoft Kinect
Software: Microsoft Visual Studio 2015, Direct2D Windows appDevelop
```

### Game Operation: 

1. **Goal**: To shoot enemy air planes, score as muchas possible within limited time, and try to avoid getting hit with enemies.
2. **How to control**: Stand in front of the Kinectcamera, keeping some distance. Open your arm as a straight line, then tilt yourarms and body to the left/right to control your airplane fly to the left/right.Or you can imagine a steering wheel to steer the air plane. The bullets will beshot out automatically from the host airplane. Stay horizontally and make yourarms stay horizontally to make your airplane stop at a spot. 
3. **End of game**: If the time is out or if any enemyplane hits your host plane, then game is over.



### Benefits of the game:

1. It gives good exercise to human body, includingbalancing, controlling for muscles, fast reactions training, faster eyemovement tracking and so on.
2. It is fun to play some games out of computer.
3. Later on, we can add features such as multipleplayers, to encourage more interactions among users over internet or local. 



### Development Approach:

1. Getting familiar with the hardware. Learningbasic components usage from Kinect, and learn Kinect SDK in C++. For example,for the data read in from the camera by using Microsoft Kinect, how can we makeuse of the located body parts (joints, head, arms and legs) of people, and howto track multiple people being seen in a scene. 

   We started by understanding that Kinectkeeps track of 25 joints of a whole body during motions, and we could getvectors to store the positions of the joints that we need to use later in thegame. Currently we use the position information of a person’s two hands todecide the arms’ slope, thus to determine which way the person is tilting, andwhich way the person wants the airplane to go. 

2. Getting started with the windows applicationdevelopment environment.  We would liketo create a user friendly interface for the players, and also want the gameapplication to be as fun as possible. We learned creating a simple Drect2Dapplication through the following website:

   [https://msdn.microsoft.com/en-us/library/windows/desktop/dd370994(v=vs.85).aspx](https://msdn.microsoft.com/en-us/library/windows/desktop/dd370994(v=vs.85).aspx)

3. Combine the data from Kinect into our gameapplication’s design was a fun experience. As described previously, we decidedto use the slope/angle as parameters to put into the windows applicationdesign.


### Future improvement

1.      We can easily extend this game into amultiplayer game by adding a split screen when Kinect detects two or morepeople. This will make local players more interactive with each other, andpotentially improves friendship etc.
2.      Different difficulty levels can easily be added,by different settings of the density of enemy aircrafts, and also by thespeed/velocity of the flying trajectories of the planes.


### Screenshot

![8](.\resource\8.png)