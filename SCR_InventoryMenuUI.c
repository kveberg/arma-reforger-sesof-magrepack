/* 
-----------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------- SESOF MagRepack -----------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------
-	  When dragging and dropping items in the Inventory UI, the Action_Drop()-method is called upon drop. This mod patches in a repack-function there.	  -
- 								If you know a better way of updatating the inventory UI, please DM me on Discord!									  -
-----------------------------------------------------------------------------------------------------------------------------------------------------------
*/

modded class SCR_InventoryMenuUI
{	
	override void MoveItemToStorageSlot()
	{

		if (!SESOF_MagRepack())
		{
			Print("Thus a call to the original method is in order.");
			super.MoveItemToStorageSlot();																		
		}
		else
			Print("Thus no call to the original method.");

	}
	
	
	bool SESOF_MagRepack()
	{
		/*
		Returns true if repacking occurred, but did not result in one new full magazine.
		Returns true when both no repacking should occur but also ...
				- Case 1: You're dragging an empty magazine.
				- Case 2: You're dropping on a full magazine.
				This helps avoid things shifting around unnecessarily.
		Otherwise, returns false if no repacking should occur. 
		*/
		
		
		/*
		CHECK IF TWO MAGS ARE INVOLVED
		*/				
		if (m_pSelectedSlotUI.GetSlotedItemFunction() != ESlotFunction.TYPE_MAGAZINE)											// are we dragging a slot with an item that is a mag?
			return false;
		
		if (m_pFocusedSlotUI.GetSlotedItemFunction() != ESlotFunction.TYPE_MAGAZINE)											// are we dropping on a slot with an item that is a mag?
			return false;
		
		Print("Selected and focused slots are magazine-slots. Fetching items in slots.");
		
		
		/*
		IF SO, GET ITEM COMPONENTS IN SELECTED AND FOCUSED SLOTS AND CHECK THESE ARE INDEED TWO DIFFERENT ITEMS
		*/
		private InventoryItemComponent fromItemEntityComponent, toItemEntityComponent
		private IEntity fromItemEntity, toItemEntity
		private bool repackOnAStack = false;																					// used later to return false when repacking on a stack, thus enabling the newly fulled mag to find a suitable slot
		
		fromItemEntityComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
		
		if (m_pSelectedSlotUI == m_pFocusedSlotUI)																				// are we dropping on the same slot?
		{
			if (m_pFocusedSlotUI.IsStacked())
			{	
				ref array<IEntity> items = {};
				GetInventoryStorageManager().GetItems(items);																	// the array will fill with items in order
				fromItemEntity = fromItemEntityComponent.GetOwner();
				int topMagPosition = items.Find(fromItemEntity);
				toItemEntity = items[topMagPosition + 1];																		// the item below in the stack is the next from the one focused amd selected
				toItemEntityComponent = m_pFocusedSlotUI.GetInventoryItemComponent();
				
				repackOnAStack = true;																		
			}
			else
			{
				return false;																										// not a stack. no repacking.
			}
		}
		
		if (m_pSelectedSlotUI != m_pFocusedSlotUI)
		{
			toItemEntityComponent = m_pFocusedSlotUI.GetInventoryItemComponent();
			fromItemEntity = fromItemEntityComponent.GetOwner();											
			toItemEntity = toItemEntityComponent.GetOwner();
		}
		
		if (fromItemEntity.GetID() == toItemEntity.GetID())																			// if we somehow have the same item at this point, no repacking should occur.
		{	
			return false;
		}
		
		
		Print(fromItemEntity);
		Print(toItemEntity);
		
		if (repackOnAStack)
			Print("Items came from same slot.");	
		else
			Print("Items came from different slots.");

		
		/*
		IF SO, GET THE MAGAZINE COMPONENTS OF THE TWO ITEMS AND SEE IF THEY SHOULD BE REPACKED
		*/
		Print("Fetching MagazineComponents.");
		private MagazineComponent fromMagazineComponent = MagazineComponent.Cast(fromItemEntity.FindComponent(MagazineComponent));	
		private MagazineComponent toMagazineComponent = MagazineComponent.Cast(toItemEntity.FindComponent(MagazineComponent));
		
		
		Print("Mag Owner:");
		Print(toMagazineComponent.GetOwner());
		
		IEntity character = SCR_EntityHelper.GetMainParent(fromMagazineComponent.GetOwner());
		Print(SCR_EntityHelper.GetMainParent(fromMagazineComponent.GetOwner()));
			

		if (fromMagazineComponent.GetMagazineWell().Type() != toMagazineComponent.GetMagazineWell().Type())							
		{
			Print("Incompatible MagazineWells.");
			return false;
		}

		if (fromMagazineComponent.GetAmmoCount() == 0)																				
		{	
			Print("There is no ammunition in the magazine you're dragging.");
			return true;
		}
		
		if (toMagazineComponent.GetAmmoCount() == toMagazineComponent.GetMaxAmmoCount())											
		{	
			Print("The magazine in the slot you're dropping on is full.");
			return true;
		}
		
		
		Print(fromMagazineComponent);
		Print(toMagazineComponent);
		Print("MagazineComponents can be repacked.");
		
		
		/*
		IF SO, REPACK MAGAZINE COMPONENTS
		*/
	
		
		private int availableAmmo = fromMagazineComponent.GetAmmoCount() + toMagazineComponent.GetAmmoCount();
		
		if (availableAmmo > toMagazineComponent.GetMaxAmmoCount())
		{
			Print("Repack should result in two magazines with ammunition in them, but only one will be full.");
			
			toMagazineComponent.SetAmmoCount(toMagazineComponent.GetMaxAmmoCount());
			fromMagazineComponent.SetAmmoCount(availableAmmo - toMagazineComponent.GetMaxAmmoCount());
			
			m_InventoryManager.PlayItemSound(toItemEntity, SCR_SoundEvent.SOUND_PICK_UP);		
					
			Print(toMagazineComponent.GetAmmoCount());
			Print(fromMagazineComponent.GetAmmoCount());
			
			// Update the widget that shows the amount of ammo in the mags.
			m_pSelectedSlotUI.SetSlotVisible(false);
			m_pSelectedSlotUI.SetSlotVisible(true);
			m_pFocusedSlotUI.SetSlotVisible(false);
			m_pFocusedSlotUI.SetSlotVisible(true);

			// Return false to allow the MoveItemToStorageSlot() to shift things around, and thus move the now filled mag to the appropriate stack if it exists.
			return false;
		}
		
		
		if (availableAmmo <= toMagazineComponent.GetMaxAmmoCount())
		{
			Print("Repack should result in one magazine with ammunition in it, and one that is empty.");
			
			toMagazineComponent.SetAmmoCount(availableAmmo);
			fromMagazineComponent.SetAmmoCount(0);
			m_InventoryManager.PlayItemSound(toItemEntity, SCR_SoundEvent.SOUND_PICK_UP);
				
			// Update the widget that shows the amount of ammo in the mags.
			m_pSelectedSlotUI.SetSlotVisible(false);
			m_pSelectedSlotUI.SetSlotVisible(true);
			m_pFocusedSlotUI.SetSlotVisible(false);
			m_pFocusedSlotUI.SetSlotVisible(true);
			
			if (repackOnAStack)
			{
				Print("Repack on a stack occurred.");
				return false;
			}
			else
				return true;
		}
		
		Print("If you see this, there is a way to get to the repacking-portion of the code that is not currently handled.");
		return false;
	}
};
