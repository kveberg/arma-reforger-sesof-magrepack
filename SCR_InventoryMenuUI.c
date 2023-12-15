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
		bool isToSlotStacked = m_pFocusedSlotUI.IsStacked();
		
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
		else
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
		
		Print(string.Format("From Mag has %1\nTo Mag Has %1", fromMagazineComponent.GetAmmoCount(), toMagazineComponent.GetAmmoCount()));
		
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
		
		// These variables are used later, and must be created here and not later.
		int fromCount = fromMagazineComponent.GetAmmoCount();
		int toCount = toMagazineComponent.GetAmmoCount();
		int maxCount = toMagazineComponent.GetMaxAmmoCount();
				
		// If either mags are at max capacity just ignore
		if(fromCount == maxCount || toCount == maxCount)
		{				
			Print("One or both magazines are full -- ignoring");
			return false;
		}
		
		protected SCR_PlayerController m_PlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(GetPlayerId()));
		RplComponent rpl = RplComponent.Cast(fromMagazineComponent.GetOwner().FindComponent(RplComponent));
		
		if(rpl.IsMaster())
			m_PlayerController.CombineMags(fromMagazineComponent, toMagazineComponent, m_InventoryManager);
		else
			m_PlayerController.Rpc(m_PlayerController.RpcAsk_CombineMags, 
				Replication.FindId(fromMagazineComponent), 
				Replication.FindId(toMagazineComponent),
				Replication.FindId(m_InventoryManager)
			);
		
		RefreshUISlotStorages();

		m_InventoryManager.PlayItemSound(toItemEntity, SCR_SoundEvent.SOUND_PICK_UP);
		
		return true;
	}	
};
