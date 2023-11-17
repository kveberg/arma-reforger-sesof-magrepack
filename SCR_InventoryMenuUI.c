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
			Print("Let's try to move it");
			super.MoveItemToStorageSlot();
		RefreshUISlotStorages();
	

	}
	
	
	
	static int GetPlayerId()
	{
		IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();
		if(!playerEntity) return 0;
		
		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);		
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
		// are we dragging a slot with an item that is a mag?
		if (m_pSelectedSlotUI.GetSlotedItemFunction() != ESlotFunction.TYPE_MAGAZINE ||
			m_pFocusedSlotUI.GetSlotedItemFunction() != ESlotFunction.TYPE_MAGAZINE)								
			return false;
		
		/*
			IF SO, GET ITEM COMPONENTS IN SELECTED AND FOCUSED SLOTS AND CHECK THESE ARE INDEED TWO DIFFERENT ITEMS
		*/
		InventoryItemComponent fromItemEntityComponent, toItemEntityComponent
		IEntity fromItemEntity, toItemEntity
		
		// used later to return false when repacking on a stack, thus enabling the newly fulled mag to find a suitable slot
		bool repackOnAStack = false;	
		bool repackFromAStack = m_pSelectedSlotUI.IsStacked();																		
		
		fromItemEntityComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
		
		// are we dropping on the same slot?
		if (m_pSelectedSlotUI == m_pFocusedSlotUI)																				
		{
			if (m_pFocusedSlotUI.IsStacked())
			{	
				ref array<IEntity> items = {};
				
				// the array will fill with items in order
				GetInventoryStorageManager().GetItems(items);
				fromItemEntity = fromItemEntityComponent.GetOwner();
				int topMagPosition = items.Find(fromItemEntity);
				
				// the item below in the stack is the next from the one focused amd selected
				toItemEntity = items[topMagPosition + 1];
				toItemEntityComponent = m_pFocusedSlotUI.GetInventoryItemComponent();
				
				repackOnAStack = true;																		
			}			
			
			// not a stack. no repacking.
			else
			{
				return true;																										
			}																									
		}
		
		if (m_pSelectedSlotUI != m_pFocusedSlotUI)
		{
			toItemEntityComponent = m_pFocusedSlotUI.GetInventoryItemComponent();
			fromItemEntity = fromItemEntityComponent.GetOwner();											
			toItemEntity = toItemEntityComponent.GetOwner();
		}
		
		// if we somehow have the same item at this point, no repacking should occur.
		if (fromItemEntity.GetID() == toItemEntity.GetID())																			
			return false;
		
		/*
			IF SO, GET THE MAGAZINE COMPONENTS OF THE TWO ITEMS AND SEE IF THEY SHOULD BE REPACKED
		*/
		MagazineComponent fromMagazineComponent = MagazineComponent.Cast(fromItemEntity.FindComponent(MagazineComponent));	
		MagazineComponent toMagazineComponent = MagazineComponent.Cast(toItemEntity.FindComponent(MagazineComponent));
		
		IEntity character = SCR_EntityHelper.GetMainParent(fromItemEntity);
			
		if (fromMagazineComponent.GetMagazineWell().Type() != toMagazineComponent.GetMagazineWell().Type())							
		{
			Print("Incompatible MagazineWells.");
			return false;
		}
		
		// If the from magazine has been consumed we should delete the magazine to clean things up
		if(fromMagazineComponent.GetAmmoCount() == 0)
		{
			Print("There is no ammunition in the magazine you're dragging.");
			SCR_EntityHelper.DeleteEntityAndChildren(fromItemEntity);
			return true;
		}
				
		if (toMagazineComponent.GetAmmoCount() == toMagazineComponent.GetMaxAmmoCount())											
		{	
			Print("The magazine in the slot you're dropping on is full.");
			if (fromMagazineComponent.GetAmmoCount() == fromMagazineComponent.GetMaxAmmoCount())
				return false;
			else
				return true;
		}
		
		// These variables are used later, and must be created here and not later.
		int fromCount = fromMagazineComponent.GetAmmoCount();
		int toCount = toMagazineComponent.GetAmmoCount();
		int maxCount = toMagazineComponent.GetMaxAmmoCount();
		
		protected SCR_PlayerController m_PlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(GetPlayerId()));
		RplComponent rpl = RplComponent.Cast(fromMagazineComponent.GetOwner().FindComponent(RplComponent));
		
		if(rpl.IsMaster())
			m_PlayerController.CombineMags(fromMagazineComponent, toMagazineComponent);
		else
			m_PlayerController.Rpc(m_PlayerController.RpcAsk_CombineMags, Replication.FindId(fromMagazineComponent), Replication.FindId(toMagazineComponent));
		
		RefreshUISlotStorages();

		m_InventoryManager.PlayItemSound(toItemEntity, SCR_SoundEvent.SOUND_PICK_UP);
		
		
		/* At this point, it would be nice to just return true. However ... 
		There are instances where the original method should run to move things and there are instances where it should not. A smooth experience depends upon getting that right.
		Another problem occurs when repack is conducted across storages, and in particular when references get changed / destroyed along the way. This affects only the 
		widget that shows the ammoCount of each mag, and that widget is refreshed in the SetSlotVisible()-method. Therefore, run that when there is indeed a widget to run it on.
		*/
		
		if (m_pSelectedSlotUI)
			m_pSelectedSlotUI.SetSlotVisible(true);
		if (m_pFocusedSlotUI)
			m_pFocusedSlotUI.SetSlotVisible(true);
		
		//Print("From:");
		//Print(fromCount);
		//Print("To:");
		//Print(toCount);
		//Print("Total:");
		//Print(fromCount + toCount);		
		
		if((fromCount + toCount) <= maxCount)
		{
			//Print("All of toMag goes to fromMag");
			return true;
		}
		if((fromCount + toCount) > maxCount)
		{
			//when there is overfill, and you're dragging a mag from a stack of N-full ones to another in another storage, it will return to the stack with wrong ammo count on top.
			if (repackFromAStack)
			{
				m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
				m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();
				BaseInventoryStorageComponent pStorageFromComponent = m_pCallBack.m_pStorageFrom.GetCurrentNavigationStorage();
				BaseInventoryStorageComponent pStorageToComponent = m_pFocusedSlotUI.GetAsStorage();
				
				m_InventoryManager.InsertItem( fromItemEntity, pStorageToComponent, pStorageFromComponent, m_pCallBack );
			}
			//Print("toMag fills up, and what remains in fromMag is:");
			//Print((fromCount + toCount) - maxCount);
			return false;
		}
		//Print("If you see this you're in trouble.");
		return false;
	}	
};
