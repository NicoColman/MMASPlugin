# MMASPlugin

MMASPlugin is designed to provide a streamlined setup for a complete locomotion system, based on Unreal’s Game Animation Sample. This plugin simplifies the process of integrating motion matching and other advanced animation features into your project, using a set of easy-to-use Blueprint or C++ functions.

### Key Features:
- **Easy Setup:** Integrate a full locomotion system with minimal effort.
- **Flexible Integration:** Add the system to any project, whether you're using Blueprint or C++.
- **Motion Matching:** Built upon Unreal's Game Animation Sample, bringing advanced animation techniques to your project.

### Disclaimer:
I originally was planning on integrating the Motion Matching system into my project. I decided to turn it into a plugin to share with the community. Please note, I am an intermediate programmer, so any feedback or improvements are welcome!

### Contribution:
If you have any suggestions, improvements, or run into any issues, feel free to share them. Your input will help make this plugin better.

Hope you find this plugin helpful!


## How to set up de plugin.

1. **Clone and Install the Plugin**

   - Navigate to your project's root folder. If a "Plugins" folder doesn't already exist, create one.

     ![image](https://github.com/user-attachments/assets/00b7b411-cb63-4ee0-b30c-bec46b04ad60)

   - Drag and drop the downloaded plugin into the "Plugins" folder.

     ![image](https://github.com/user-attachments/assets/5d7a2d0e-6948-4a4b-948b-1f1dc70c497d)

   - Launch your project. When prompted to rebuild the project, click "Yes" to continue.

Here’s an improved version of the second setup instruction:

2. **Adding the Traversal Actor Component**

   - Open your character's Blueprint. If you're using the third-person template, this will likely be "BP_ThirdPerson."

   - In the Components tab, click **Add** and search for `MMAS_ACTraversal`.

     ![image](https://github.com/user-attachments/assets/0be81554-2466-4c2e-9ff3-100be1d6378d)

   - In the Details panel, under the **EANNAMMAS | Traversal** section, locate **TraversalMontagesChooserTable** and assign it the UAsset `CHT_Traversal`.

     ![image](https://github.com/user-attachments/assets/f8ee2710-7bc7-4422-b137-84f779a5c3bd)

   - If you don't see the asset, make sure **Show Plugin Content** is enabled.

     ![image](https://github.com/user-attachments/assets/d5f4174d-b9cc-4f1d-b3e1-4fc5811b1db9)

   - Additionally, the **Debug** category is available for monitoring and debugging traversal-related variables.

3. **Adding the Character Interface**

   The Character Interface is crucial for the plugin's functionality. Forgetting to implement it will cause the engine to crash (intentionally, to remind you).

   - In the Class Settings, under **Implemented Interfaces**, add `MMAS_InterfaceCharacter`.

   - The necessary functions will appear in the **Interfaces** panel.
     
     ![image](https://github.com/user-attachments/assets/126b4ba2-0a8d-4fef-8d9a-7a9d9db3364a)

     - **TryTraversalAction**
       - This function is used when the **Jump** event is triggered in another class, such as a **GameplayAbility**. In this example, we will not cover this function.

     - **GetGait**
       - This function is used by the AnimInstance to check whether the character is walking or running. You should override this function to fit your walking/running logic.
       ![image](https://github.com/user-attachments/assets/dac9b891-5716-4dfa-ad2c-42425d76fbc6)

     - **GetCharacterMotionWarpingComponent**
       - This function retrieves the **MotionWarpingComponent** used in `MMAS_ACTraversal`. It must be overridden.
       - Be sure to add the **MotionWarpingComponent** to your character.
       ![image](https://github.com/user-attachments/assets/fe7beb33-4037-4315-bbb5-c4d978a3dbb5)
       ![image](https://github.com/user-attachments/assets/01e57164-234e-48ce-97a2-1d814d19d754)

     - **GetCharacterJumpData**
       - This function allows the AnimInstance to determine whether the character should perform a light or heavy landing. You need to override this function.
       - Create a new variable of type `CharacterJumpData`.
       ![image](https://github.com/user-attachments/assets/f5e3185f-f68b-4b5b-950e-3a8435156bff)

       - In the Event Graph, add the **OnLanded** event and set the **CharacterJumpData** members like this:
       ![image](https://github.com/user-attachments/assets/6cfc5037-8eba-4bac-bb5a-f134a91cf32a)

4. **Event Jump**

   - In the **IA_Jump** action, call the function `PerformJumpOrTraversalAction` from the `MMASACTraversal` component.

     ![image](https://github.com/user-attachments/assets/38ae32c9-ffc1-4087-9026-7a6417584025)

5. **Select the AnimInstance**

   - Set **ABP_Character_3P** as the AnimInstance Class and **SKM_UEFN_Mannequin** as the Skeleton.
     
     ![image](https://github.com/user-attachments/assets/ce3ea38a-ed7e-4341-8a86-d0ccd42ecbef)

6. **Traversal Blocks**

   - In `EannaMMASContent/Blueprint/TraversalActor`, drag **BP_Traversal** into the world. This actor can be adjusted for different types of traversal actions depending on its scale. It works in conjunction with the `IMMAS_ActorTraversalInterface`.
     
     ![image](https://github.com/user-attachments/assets/fff10e70-0a33-4652-aaf3-76b408928a2a)

---

**Plugin setup complete!**

---

## Further Optimization

A. **Jump Character Movement**

   - You may notice that when the character jumps, if you stop moving mid-air, it will fall straight down. To make the jump behavior similar to the Game Animation Sample, adjust the following values in **CharacterMovement** inside **BP_ThirdPerson**:

     ![image](https://github.com/user-attachments/assets/c86318d6-5675-402f-98a8-4dff50009a18)

---

## Questions?

If you have any questions or need assistance, feel free to reach out—I'll be more than happy to help!

   

     

   

   
